// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER


#include "Thyra_EpetraExtAddTransformer.hpp"
#include "Thyra_AddedLinearOpBase.hpp"
#include "Thyra_ScaledAdjointLinearOpBase.hpp"
#include "Thyra_EpetraLinearOp.hpp"
#include "Thyra_get_Epetra_Operator.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Epetra_Map.h"
#include "Epetra_LocalMap.h"
#include "Epetra_SerialComm.h"
#include "Epetra_CrsMatrix.h"
#include "Teuchos_Assert.hpp"
#include "EpetraExt_ConfigDefs.h"
#include "EpetraExt_MatrixMatrix.h"
#include "EpetraExt_MMHelpers.h"
#include "EpetraExt_Transpose_RowMatrix.h"


#include "EpetraExt_RowMatrixOut.h"


namespace Thyra {


// Overridden from LinearOpTransformerBase


bool EpetraExtAddTransformer::isCompatible(
      const LinearOpBase<double> &op_in) const
{
   TEST_FOR_EXCEPT(true);
   return false;
}


RCP<LinearOpBase<double> >
EpetraExtAddTransformer::createOutputOp() const
{
   return nonconstEpetraLinearOp();
}


void EpetraExtAddTransformer::transform(
   const LinearOpBase<double> &op_in,
   const Ptr<LinearOpBase<double> > &op_inout) const
{
   using Thyra::unwrap;
   using EpetraExt::MatrixMatrix;
   using Teuchos::rcp;
   using Teuchos::rcp_dynamic_cast;
   using Teuchos::dyn_cast;
 
   //
   // A) Get the component Thyra objects for M = op(B) * D * G
   //
 
   const AddedLinearOpBase<double> & add_op =
         dyn_cast<const AddedLinearOpBase<double> >(op_in);

#ifdef TEUCHOS_DEBUG
   TEUCHOS_ASSERT_EQUALITY( add_op.numOps(), 2 );
#endif

 
   // get properties of first operator: Transpose, scaler multiply...etc
   const RCP<const LinearOpBase<double> > op_A = add_op.getOp(0);
   double A_scalar = 0.0;
   EOpTransp A_transp = NOTRANS;
   RCP<const LinearOpBase<double> > A;
   unwrap( op_A, &A_scalar, &A_transp, &A );
   TEUCHOS_ASSERT(A_transp==NOTRANS || A_transp==CONJTRANS); // sanity check
 
   // get properties of third operator: Transpose, scaler multiply...etc
   const RCP<const LinearOpBase<double> > op_B = add_op.getOp(1);
   double B_scalar = 0.0;
   EOpTransp B_transp = NOTRANS;
   RCP<const LinearOpBase<double> > B;
   unwrap( op_B, &B_scalar, &B_transp, &B );
   TEUCHOS_ASSERT(B_transp==NOTRANS || B_transp==CONJTRANS); // sanity check

   //
   // B) Extract out the Epetra_CrsMatrix objects and the vector
   //
   
  // convert operators to Epetra_CrsMatrix
   const RCP<const Epetra_CrsMatrix> epetra_A =
         rcp_dynamic_cast<const Epetra_CrsMatrix>(get_Epetra_Operator(*A), true);
   const RCP<const Epetra_CrsMatrix> epetra_B =
         rcp_dynamic_cast<const Epetra_CrsMatrix>(get_Epetra_Operator(*B), true);
   
   const Epetra_Map op_inout_row_map 
         = (A_transp==CONJTRANS ? epetra_A->ColMap() : epetra_A->RowMap());
   const Epetra_Map op_inout_col_map 
         = (A_transp==CONJTRANS ? epetra_A->RowMap() : epetra_A->ColMap());
 
   //
   // C) Do the explicit addition
   //
  
   // allocate space for final addition: 3 steps
   //   1. Get destination EpetraLinearOp
   //   2. Extract RCP to destination Epetra_CrsMatrix
   //   3. If neccessary, allocate new Epetra_CrsMatrix
   EpetraLinearOp &thyra_epetra_op_inout = dyn_cast<EpetraLinearOp>(*op_inout);
   RCP<Epetra_CrsMatrix>  epetra_op =
         rcp_dynamic_cast<Epetra_CrsMatrix>(thyra_epetra_op_inout.epetra_op());
   Epetra_CrsMatrix * epetra_op_raw = epetra_op.get();

   // perform addition
   const int add_epetra_B_err 
      = EpetraExt::MatrixMatrix::Add(*epetra_A,A_transp==CONJTRANS,A_scalar,*epetra_B,B_transp==CONJTRANS,B_scalar,epetra_op_raw);
   if(epetra_op==Teuchos::null)
      epetra_op = Teuchos::rcp(epetra_op_raw);
   
   TEUCHOS_ASSERT_EQUALITY( add_epetra_B_err, 0 );
   
   epetra_op->FillComplete();

   // set output operator to use newly create epetra_op
   thyra_epetra_op_inout.initialize(epetra_op);
}


} // namespace Thyra

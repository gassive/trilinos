// @HEADER
// ***********************************************************************
// 
// Moocho: Multi-functional Object-Oriented arCHitecture for Optimization
//                  Copyright (2003) Sandia Corporation
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
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#include <assert.h>

#include <typeinfo>

#include "ConstrainedOptPack_DecompositionSystemOrthogonal.hpp"
#include "ConstrainedOptPack_MatrixIdentConcatStd.hpp"
#include "ConstrainedOptPack_MatrixDecompRangeOrthog.hpp"
#include "AbstractLinAlgPack_MatrixSymOpNonsing.hpp"
#include "AbstractLinAlgPack_MatrixSymInitDiag.hpp"
#include "AbstractLinAlgPack_MatrixComposite.hpp"
#include "AbstractLinAlgPack_MatrixOpSubView.hpp"
#include "AbstractLinAlgPack_LinAlgOpPack.hpp"
#include "Teuchos_AbstractFactoryStd.hpp"
#include "Teuchos_dyn_cast.hpp"
#include "Teuchos_TestForException.hpp"

namespace ConstrainedOptPack {

DecompositionSystemOrthogonal::DecompositionSystemOrthogonal(
  const VectorSpace::space_ptr_t           &space_x
  ,const VectorSpace::space_ptr_t          &space_c
  ,const basis_sys_ptr_t                   &basis_sys
  ,const basis_sys_tester_ptr_t            &basis_sys_tester
  ,EExplicitImplicit                       D_imp
  ,EExplicitImplicit                       Uz_imp
  )
  :DecompositionSystemVarReductImp(
    space_x, space_c, basis_sys, basis_sys_tester
    ,D_imp, Uz_imp )
{}

// Overridden from DecompositionSystem

const DecompositionSystem::mat_fcty_ptr_t
DecompositionSystemOrthogonal::factory_Y() const
{
  namespace rcp = MemMngPack;
  return Teuchos::rcp(
    new Teuchos::AbstractFactoryStd<MatrixOp,MatrixIdentConcatStd>()
    );
}

const DecompositionSystem::mat_nonsing_fcty_ptr_t
DecompositionSystemOrthogonal::factory_R() const
{
  namespace rcp = MemMngPack;
  return Teuchos::rcp(
    new Teuchos::AbstractFactoryStd<MatrixOpNonsing,MatrixDecompRangeOrthog>()
    );
}

const DecompositionSystem::mat_fcty_ptr_t
DecompositionSystemOrthogonal::factory_Uy() const
{
  return Teuchos::rcp(	new Teuchos::AbstractFactoryStd<MatrixOp,MatrixOpSubView>() );
}

// Overridden from DecompositionSystemVarReductImp

void DecompositionSystemOrthogonal::update_D_imp_used(EExplicitImplicit *D_imp_used) const
{
  *D_imp_used = MAT_IMP_EXPLICIT;
}

DecompositionSystem::mat_nonsing_fcty_ptr_t::element_type::obj_ptr_t
DecompositionSystemOrthogonal::uninitialize_matrices(
  std::ostream                                       *out
  ,EOutputLevel                                      olevel
  ,MatrixOp                                          *Y
  ,MatrixOpNonsing                                   *R
  ,MatrixOp                                          *Uy
  ) const
{
  namespace rcp = MemMngPack;
  using Teuchos::dyn_cast;
  typedef DecompositionSystem::mat_nonsing_fcty_ptr_t::element_type::obj_ptr_t
    C_ptr_t;

  //
  // Get pointers to concreate matrices
  //
  
  MatrixIdentConcatStd
    *Y_orth = Y ? &dyn_cast<MatrixIdentConcatStd>(*Y)    : NULL;
  MatrixDecompRangeOrthog
    *R_orth = R ? &dyn_cast<MatrixDecompRangeOrthog>(*R) : NULL;
  MatrixOpSubView
    *Uy_cpst = Uy ? &dyn_cast<MatrixOpSubView>(*Uy)  : NULL;			

  //
  // Get the smart pointer to the basis matrix object C and the
  // matrix S = I + D'*D
  //
  
  C_ptr_t C_ptr = Teuchos::null;
  if(R_orth) {
    C_ptr  = Teuchos::rcp_const_cast<MatrixOpNonsing>(    R_orth->C_ptr() ); // This could be NULL!
    S_ptr_ = Teuchos::rcp_const_cast<MatrixSymOpNonsing>( R_orth->S_ptr() ); // ""
  }
  
  //
  // Uninitialize all of the matrices to remove references to C, D etc.
  //

  if(Y_orth)
    Y_orth->set_uninitialized();
  if(R_orth)
    R_orth->set_uninitialized();
  if(Uy_cpst)
    Uy_cpst->initialize(Teuchos::null);

  //
  // Return the owned? basis matrix object C
  //

  return C_ptr;

}

void DecompositionSystemOrthogonal::initialize_matrices(
  std::ostream                                           *out
  ,EOutputLevel                                          olevel
  ,const mat_nonsing_fcty_ptr_t::element_type::obj_ptr_t &C
  ,const mat_fcty_ptr_t::element_type::obj_ptr_t         &D
  ,MatrixOp                                              *Y
  ,MatrixOpNonsing                                       *R
  ,MatrixOp                                              *Uy
  ,EMatRelations                                         mat_rel
  ) const
{
  namespace rcp = MemMngPack;
  using Teuchos::dyn_cast;
  using LinAlgOpPack::syrk;
  typedef DecompositionSystem::mat_nonsing_fcty_ptr_t::element_type::obj_ptr_t
    C_ptr_t;
  typedef DecompositionSystem::mat_fcty_ptr_t::element_type::obj_ptr_t
    D_ptr_t;

  const size_type
    //n = this->n(),
    r = this->r();
  const Range1D
    var_dep(1,r);

  //
  // Get pointers to concreate matrices
  //
  
  MatrixIdentConcatStd
    *Y_orth = Y ? &dyn_cast<MatrixIdentConcatStd>(*Y)    : NULL;
  MatrixDecompRangeOrthog
    *R_orth = R ? &dyn_cast<MatrixDecompRangeOrthog>(*R) : NULL;

  //
  // Initialize the matrices
  //

  if(Y_orth) {
    D_ptr_t  D_ptr = D;
    if(mat_rel == MATRICES_INDEP_IMPS) {
      D_ptr = D->clone();
      TEST_FOR_EXCEPTION(
        D_ptr.get() == NULL, std::logic_error
        ,"DecompositionSystemOrthogonal::update_decomp(...) : Error, "
        "The matrix class used for the direct sensitivity matrix D = inv(C)*N of type \'"
        << typeName(*D) << "\' must return return.get() != NULL from the clone() method "
        "since mat_rel == MATRICES_INDEP_IMPS!" );
    }
    Y_orth->initialize(
      space_x()                                         // space_cols
      ,space_x()->sub_space(var_dep)->clone()           // space_rows
      ,MatrixIdentConcatStd::BOTTOM                     // top_or_bottom
      ,-1.0                                             // alpha
      ,D_ptr                                            // D_ptr
      ,BLAS_Cpp::trans                                  // D_trans
      );
  }
  if(R_orth) {
    C_ptr_t  C_ptr = C;
    if(mat_rel == MATRICES_INDEP_IMPS) {
      C_ptr = C->clone_mwons();
      TEST_FOR_EXCEPTION(
        C_ptr.get() == NULL, std::logic_error
        ,"DecompositionSystemOrthogonal::update_decomp(...) : Error, "
        "The matrix class used for the basis matrix C of type \'"
        << typeName(*C) << "\' must return return.get() != NULL from the clone_mwons() method "
        "since mat_rel == MATRICES_INDEP_IMPS!" );
    }
    D_ptr_t  D_ptr = D;
    if(mat_rel == MATRICES_INDEP_IMPS) {
      D_ptr = D->clone();
      TEST_FOR_EXCEPTION(
        D_ptr.get() == NULL, std::logic_error
        ,"DecompositionSystemOrthogonal::update_decomp(...) : Error, "
        "The matrix class used for the direct sensitivity matrix D = inv(C)*N of type \'"
        << typeName(*D) << "\' must return return.get() != NULL from the clone() method "
        "since mat_rel == MATRICES_INDEP_IMPS!" );
    }
    if(S_ptr_.get() == NULL) {
      S_ptr_ = basis_sys()->factory_S()->create();
    }
    try {
      // S = I + (D)'*(D')'
      dyn_cast<MatrixSymInitDiag>(*S_ptr_).init_identity(D_ptr->space_rows());
      syrk(*D_ptr,BLAS_Cpp::trans,1.0,1.0,S_ptr_.get());
    }
    catch( const MatrixNonsing::SingularMatrix& except ) {
      TEST_FOR_EXCEPTION(
        true, SingularDecomposition
        ,"DecompositionSystemOrthogonal::initialize_matrices(...) : Error, update of S failed : "
        << except.what() );
    }
    R_orth->initialize(C_ptr,D_ptr,S_ptr_);
  }
  // ToDo: Implement for undecomposed equalities and general inequalities
}

void DecompositionSystemOrthogonal::print_update_matrices(
  std::ostream& out, const std::string& L ) const
{
  out
    << L << "*** Orthogonal decompositon Y, R and Uy matrices (class DecompositionSystemOrthogonal)\n"
    << L << "Y  = [ I; -D' ] (using class MatrixIdentConcatStd)\n"
    << L << "R  = C*(I + D*D')\n"
    << L << "Uy = E - F*D'\n"
    ;
}
    
}	// end namespace ConstrainedOptPack

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

#include "Thyra_DiagonalEpetraLinearOpWithSolveFactory.hpp"
#include "Thyra_DefaultDiagonalLinearOpWithSolve.hpp"
#include "Thyra_EpetraLinearOp.hpp"
#include "Thyra_EpetraThyraWrappers.hpp"
#include "Teuchos_dyn_cast.hpp"

#include "Epetra_RowMatrix.h"
#include "Epetra_Vector.h"
#include "Epetra_Map.h"


namespace Thyra {


bool DiagonalEpetraLinearOpWithSolveFactory::isCompatible(
  const LinearOpSourceBase<double> &fwdOpSrc
  ) const
{
  using Teuchos::outArg;
  RCP<const LinearOpBase<double> >
    fwdOp = fwdOpSrc.getOp();
  const EpetraLinearOpBase *eFwdOp = NULL;
  if( ! (eFwdOp = dynamic_cast<const EpetraLinearOpBase*>(&*fwdOp)) )
    return false;
  RCP<const Epetra_Operator> epetraFwdOp;
  EOpTransp epetraFwdOpTransp;
  EApplyEpetraOpAs epetraFwdOpApplyAs;
  EAdjointEpetraOp epetraFwdOpAdjointSupport;
  eFwdOp->getEpetraOpView(outArg(epetraFwdOp), outArg(epetraFwdOpTransp),
    outArg(epetraFwdOpApplyAs), outArg(epetraFwdOpAdjointSupport) );
  if( !dynamic_cast<const Epetra_RowMatrix*>(&*epetraFwdOp) )
    return false;
  return true;
}


RCP<LinearOpWithSolveBase<double> >
DiagonalEpetraLinearOpWithSolveFactory::createOp() const
{
  return Teuchos::rcp(new DefaultDiagonalLinearOpWithSolve<double>());
}


void DiagonalEpetraLinearOpWithSolveFactory::initializeOp(
  const RCP<const LinearOpSourceBase<double> >    &fwdOpSrc
  ,LinearOpWithSolveBase<double>                                   *Op
  ,const ESupportSolveUse                                          supportSolveUse
  ) const
{
  using Teuchos::outArg;
  TEST_FOR_EXCEPT(Op==NULL);
  TEST_FOR_EXCEPT(fwdOpSrc.get()==NULL);
  TEST_FOR_EXCEPT(fwdOpSrc->getOp().get()==NULL);
  RCP<const LinearOpBase<double> > fwdOp = fwdOpSrc->getOp();
  const EpetraLinearOpBase &eFwdOp = Teuchos::dyn_cast<const EpetraLinearOpBase>(*fwdOp);
  RCP<const Epetra_Operator> epetraFwdOp;
  EOpTransp epetraFwdOpTransp;
  EApplyEpetraOpAs epetraFwdOpApplyAs;
  EAdjointEpetraOp epetraFwdOpAdjointSupport;
  eFwdOp.getEpetraOpView(outArg(epetraFwdOp), outArg(epetraFwdOpTransp),
    outArg(epetraFwdOpApplyAs), outArg(epetraFwdOpAdjointSupport) );
  const Epetra_RowMatrix &eRMOp  =
    Teuchos::dyn_cast<const Epetra_RowMatrix>(*epetraFwdOp);
  const Epetra_Map &map = eRMOp.OperatorDomainMap();
  RCP<Epetra_Vector>
    e_diag = Teuchos::rcp(new Epetra_Vector(map));
  eRMOp.ExtractDiagonalCopy(*e_diag);
  RCP< const VectorSpaceBase<double> >
    space = create_VectorSpace(Teuchos::rcp(new Epetra_Map(map)));
  RCP< const VectorBase<double> >
    diag = create_Vector(e_diag,space);
  Teuchos::set_extra_data<RCP<const LinearOpSourceBase<double> > >(
    fwdOpSrc, "Thyra::DiagonalEpetraLinearOpWithSolveFactory::fwdOpSrc",
    Teuchos::inOutArg(diag)
    );
  Teuchos::dyn_cast< DefaultDiagonalLinearOpWithSolve<double> >(*Op).initialize(
    Teuchos::rcp_implicit_cast<const VectorBase<double> >(diag)
    );
  // Above cast is questionable but should be okay based on use.
}


void DiagonalEpetraLinearOpWithSolveFactory::uninitializeOp(
  LinearOpWithSolveBase<double>                               *Op
  ,RCP<const LinearOpSourceBase<double> >    *fwdOpSrc
  ,RCP<const PreconditionerBase<double> >    *prec
  ,RCP<const LinearOpSourceBase<double> >    *approxFwdOpSrc
  ,ESupportSolveUse                                           *supportSolveUse
  ) const
{
  using Teuchos::get_extra_data;
  TEST_FOR_EXCEPT(Op==NULL);
  DefaultDiagonalLinearOpWithSolve<double>
    &diagOp = Teuchos::dyn_cast<DefaultDiagonalLinearOpWithSolve<double> >(*Op);
  RCP< const VectorBase<double> >
    diag = diagOp.getDiag();
  if( fwdOpSrc ) {
    if(diag.get()) {
      *fwdOpSrc =
        get_extra_data<RCP<const LinearOpSourceBase<double> > >(
          diag,"Thyra::DiagonalEpetraLinearOpWithSolveFactory::fwdOpSrc"
          );
    }
  }
  else {
    *fwdOpSrc = Teuchos::null;
  }
  if(prec) *prec = Teuchos::null; // We never keep a preconditioner!
  if(approxFwdOpSrc) *approxFwdOpSrc = Teuchos::null; // We never keep a preconditioner!
}


// Overridden from ParameterListAcceptor


void DiagonalEpetraLinearOpWithSolveFactory::setParameterList(
  RCP<Teuchos::ParameterList> const& paramList
  )
{}


RCP<Teuchos::ParameterList>
DiagonalEpetraLinearOpWithSolveFactory::getNonconstParameterList()
{
  return Teuchos::null;
}


RCP<Teuchos::ParameterList>
DiagonalEpetraLinearOpWithSolveFactory::unsetParameterList()
{
  return Teuchos::null;
}


RCP<const Teuchos::ParameterList>
DiagonalEpetraLinearOpWithSolveFactory::getParameterList() const
{
  return Teuchos::null;
}


RCP<const Teuchos::ParameterList>
DiagonalEpetraLinearOpWithSolveFactory::getValidParameters() const
{
  return Teuchos::null;
}


} // namespace Thyra

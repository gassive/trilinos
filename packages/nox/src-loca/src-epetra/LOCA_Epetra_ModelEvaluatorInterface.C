// $Id$ 
// $Source$ 

//@HEADER
// ************************************************************************
// 
//            NOX: An Object-Oriented Nonlinear Solver Package
//                 Copyright (2002) Sandia Corporation
// 
//            LOCA: Library of Continuation Algorithms Package
//                 Copyright (2005) Sandia Corporation
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
// 
// Questions? Contact Roger Pawlowski (rppawlo@sandia.gov) or 
// Eric Phipps (etphipp@sandia.gov), Sandia National Laboratories.
// ************************************************************************
//  CVS Information
//  $Source$
//  $Author$
//  $Date$
//  $Revision$
// ************************************************************************
//@HEADER

#include "LOCA_Epetra_ModelEvaluatorInterface.H"
#include "LOCA_Parameter_Vector.H"
#include "NOX_Epetra_MultiVector.H"
#include "LOCA_MultiContinuation_AbstractGroup.H"

#include "EpetraExt_ModelEvaluator.h"
#include "Epetra_Operator.h"

// *****************************************************************
// *****************************************************************
LOCA::Epetra::ModelEvaluatorInterface::
ModelEvaluatorInterface(
		   const Teuchos::RCP<LOCA::GlobalData>& global_data,
		   const Teuchos::RefCountPtr<EpetraExt::ModelEvaluator>& m,
		   double perturb) :
  NOX::Epetra::ModelEvaluatorInterface(m),
  LOCA::DerivUtils(global_data, perturb),
  param_vec(*(m->get_p_init(0))),
  alpha_prev(0),
  beta_prev(1),
  loca_param_vec(),
  x_dot(NULL),
  observer(Teuchos::null)
{
  // Get parameter names
  Teuchos::RefCountPtr<const Teuchos::Array<std::string> > param_names =
    m->get_p_names(0);
  for (std::size_t i=0; i<param_names->size(); i++)
    loca_param_vec.addParameter((*param_names)[i], param_vec[i]);
}

// *****************************************************************
// *****************************************************************
LOCA::Epetra::ModelEvaluatorInterface::~ModelEvaluatorInterface()
{
  if (x_dot)
    delete x_dot;
}

// *****************************************************************
// *****************************************************************
const LOCA::ParameterVector&
LOCA::Epetra::ModelEvaluatorInterface::getLOCAParameterVector() const
{
  return loca_param_vec;
}

// *****************************************************************
// *****************************************************************
bool LOCA::Epetra::ModelEvaluatorInterface::
computeF(const Epetra_Vector& x, Epetra_Vector& F, const FillType fillFlag)
{
  // Create inargs
  EpetraExt::ModelEvaluator::InArgs inargs = model_->createInArgs();
  inargs.set_x(Teuchos::rcp(&x, false));
  inargs.set_p(0, Teuchos::rcp(&param_vec, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_x_dot)) {
    // Create x_dot, filled with zeros
    if (x_dot == NULL)
      x_dot = new Epetra_Vector(x.Map());
    inargs.set_x_dot(Teuchos::rcp(x_dot, false));
  }
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_alpha))
    inargs.set_alpha(0.0);
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_beta))
    inargs.set_beta(1.0);

  // Create outargs
  EpetraExt::ModelEvaluator::OutArgs outargs = model_->createOutArgs();
  EpetraExt::ModelEvaluator::Evaluation<Epetra_Vector> eval_f;
  Teuchos::RefCountPtr<Epetra_Vector> f = Teuchos::rcp(&F, false);
  if (fillFlag == NOX::Epetra::Interface::Required::Residual)
    eval_f.reset(f, EpetraExt::ModelEvaluator::EVAL_TYPE_EXACT); 
  else if (fillFlag == NOX::Epetra::Interface::Required::Jac)
    eval_f.reset(f, EpetraExt::ModelEvaluator::EVAL_TYPE_APPROX_DERIV);
  else
    eval_f.reset(f, EpetraExt::ModelEvaluator::EVAL_TYPE_VERY_APPROX_DERIV);
  outargs.set_f(eval_f);

  model_->evalModel(inargs, outargs);

  return true;
}
   
// *****************************************************************
// ***************************************************************** 
bool LOCA::Epetra::ModelEvaluatorInterface::
computeJacobian(const Epetra_Vector& x, Epetra_Operator& Jac)
{
  // Create inargs
  EpetraExt::ModelEvaluator::InArgs inargs = model_->createInArgs();
  inargs.set_x(Teuchos::rcp(&x, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_alpha))
    inargs.set_alpha(0.0);
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_beta))
    inargs.set_beta(1.0);
  alpha_prev = 0.0; beta_prev = 1.0; // prec must know alpha and beta
  inargs.set_p(0, Teuchos::rcp(&param_vec, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_x_dot)) {
    // Create x_dot, filled with zeros
    if (x_dot == NULL)
      x_dot = new Epetra_Vector(x.Map());
    inargs.set_x_dot(Teuchos::rcp(x_dot, false));
  }

  // Create outargs
  EpetraExt::ModelEvaluator::OutArgs outargs = model_->createOutArgs();
  EpetraExt::ModelEvaluator::Evaluation<Epetra_Vector> eval_f;
  outargs.set_f(eval_f);
  outargs.set_W(Teuchos::rcp(&Jac, false));

  model_->evalModel(inargs, outargs);

  return true;
}

// *****************************************************************
// *****************************************************************
bool LOCA::Epetra::ModelEvaluatorInterface::
computePreconditioner(const Epetra_Vector& x, 
		      Epetra_Operator& M,
		      Teuchos::ParameterList* precParams)
{
  // Create inargs
  EpetraExt::ModelEvaluator::InArgs inargs = model_->createInArgs();
  inargs.set_x(Teuchos::rcp(&x, false));

  // alpha and beta are stored from previous matrix computation
  // which might have been computeJacobian or computeShiftedMatrix
  // This is a state-full hack, but needed since this function
  // does not take alpha and beta as arguments. [AGS 01/10]
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_alpha))
    inargs.set_alpha(alpha_prev);
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_beta))
    inargs.set_beta(beta_prev);
  inargs.set_p(0, Teuchos::rcp(&param_vec, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_x_dot)) {
    // Create x_dot, filled with zeros
    if (x_dot == NULL)
      x_dot = new Epetra_Vector(x.Map());
    inargs.set_x_dot(Teuchos::rcp(x_dot, false));
  }

  // Create outargs
  EpetraExt::ModelEvaluator::OutArgs outargs = model_->createOutArgs();
  EpetraExt::ModelEvaluator::Evaluation<Epetra_Vector> eval_f;
  eval_f.reset(Teuchos::null, 
               EpetraExt::ModelEvaluator::EVAL_TYPE_VERY_APPROX_DERIV);
  outargs.set_f(eval_f);
  outargs.set_WPrec(Teuchos::rcp(&M, false));

  model_->evalModel(inargs, outargs);

  return true;
}

// *****************************************************************
// *****************************************************************
void LOCA::Epetra::ModelEvaluatorInterface::
setParameters(const ParameterVector& p)
{
  for (int i=0; i<p.length(); i++)
    param_vec[i] = p[i];
}

// *****************************************************************
// *****************************************************************
bool LOCA::Epetra::ModelEvaluatorInterface::
computeShiftedMatrix(double alpha, double beta, const Epetra_Vector& x,
		     Epetra_Operator& A)
{
  // Create inargs
  EpetraExt::ModelEvaluator::InArgs inargs = model_->createInArgs();
  inargs.set_x(Teuchos::rcp(&x, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_alpha))
    inargs.set_alpha(-beta); // alpha and beta are switched between LOCA and Thyra
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_beta))
    inargs.set_beta(alpha);
  alpha_prev = -beta; beta_prev = alpha; // prec must know alpha and beta
  inargs.set_p(0, Teuchos::rcp(&param_vec, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_x_dot)) {
    // Create x_dot, filled with zeros
    if (x_dot == NULL)
      x_dot = new Epetra_Vector(x.Map());
    inargs.set_x_dot(Teuchos::rcp(x_dot, false));
  }

  // Create outargs
  EpetraExt::ModelEvaluator::OutArgs outargs = model_->createOutArgs();
  EpetraExt::ModelEvaluator::Evaluation<Epetra_Vector> eval_f;
  outargs.set_f(eval_f);
  outargs.set_W(Teuchos::rcp(&A, false));

  model_->evalModel(inargs, outargs);

  return true;
}

// *****************************************************************
// *****************************************************************
void LOCA::Epetra::ModelEvaluatorInterface::
setXdot(const Epetra_Vector& xdot_, const double time_)
{
  if (x_dot == NULL)
    x_dot = new Epetra_Vector(xdot_.Map());
  *x_dot = xdot_;
}

// *****************************************************************
// *****************************************************************

void
LOCA::Epetra::ModelEvaluatorInterface::
printSolution  (const Epetra_Vector  &x_, double conParam)
{ 
  if ( !observer.is_null() )
     observer->observeSolution( x_ ); 
}

// *****************************************************************
// *****************************************************************

void
LOCA::Epetra::ModelEvaluatorInterface::
setObserver( const Teuchos::RCP<NOX::Epetra::Observer> & observer_ )
{
  observer = observer_;
}

// *****************************************************************
// *****************************************************************

LOCA::Epetra::ModelEvaluatorInterface::
ModelEvaluatorInterface(const LOCA::Epetra::ModelEvaluatorInterface& m) :
  NOX::Epetra::ModelEvaluatorInterface(m),
  LOCA::DerivUtils(m),
  param_vec(m.param_vec),
  loca_param_vec(m.loca_param_vec),
  x_dot(NULL)
{
  if (m.x_dot != NULL) {
    x_dot = new Epetra_Vector(*m.x_dot);
  }
}

Teuchos::RCP<LOCA::DerivUtils> 
LOCA::Epetra::ModelEvaluatorInterface::
clone(NOX::CopyType type) const
{
  return Teuchos::rcp(new LOCA::Epetra::ModelEvaluatorInterface(*this));
}

NOX::Abstract::Group::ReturnType 
LOCA::Epetra::ModelEvaluatorInterface::
computeDfDp(LOCA::MultiContinuation::AbstractGroup& grp, 
	    const vector<int>& param_ids,
	    NOX::Abstract::MultiVector& result,
	    bool isValidF) const
{
  // Break result into f and df/dp
  NOX::Epetra::Vector& f = dynamic_cast<NOX::Epetra::Vector&>(result[0]);
  Epetra_Vector& epetra_f = f.getEpetraVector();

  std::vector<int> dfdp_index(result.numVectors()-1);
  for (unsigned int i=0; i<dfdp_index.size(); i++)
    dfdp_index[i] = i+1;
  Teuchos::RefCountPtr<NOX::Epetra::MultiVector> dfdp =
    Teuchos::rcp_dynamic_cast<NOX::Epetra::MultiVector>(result.subView(dfdp_index));
  Epetra_MultiVector& epetra_dfdp = dfdp->getEpetraMultiVector();

  // Create inargs
  EpetraExt::ModelEvaluator::InArgs inargs = model_->createInArgs();
  const NOX::Epetra::Vector& x = 
    dynamic_cast<const NOX::Epetra::Vector&>(grp.getX());
  const Epetra_Vector& epetra_x = x.getEpetraVector();
  inargs.set_x(Teuchos::rcp(&epetra_x, false));
  inargs.set_p(0, Teuchos::rcp(&param_vec, false));
  if (inargs.supports(EpetraExt::ModelEvaluator::IN_ARG_x_dot)) {
    // Create x_dot, filled with zeros
    if (x_dot == NULL)
      x_dot = new Epetra_Vector(epetra_x.Map());
    inargs.set_x_dot(Teuchos::rcp(x_dot, false));
  }

  // Create outargs
  EpetraExt::ModelEvaluator::OutArgs outargs = model_->createOutArgs();
  if (!isValidF) {
    EpetraExt::ModelEvaluator::Evaluation<Epetra_Vector> eval_f;
    Teuchos::RefCountPtr<Epetra_Vector> F = Teuchos::rcp(&epetra_f, false);
    eval_f.reset(F, EpetraExt::ModelEvaluator::EVAL_TYPE_EXACT); 
    outargs.set_f(eval_f);
  }
  Teuchos::RefCountPtr<Epetra_MultiVector> DfDp = 
    Teuchos::rcp(&epetra_dfdp, false);
  Teuchos::Array<int> param_indexes(param_ids.size());
  for (unsigned int i=0; i<param_ids.size(); i++)
    param_indexes[i] = param_ids[i];
  EpetraExt::ModelEvaluator::DerivativeMultiVector dmv(DfDp, EpetraExt::ModelEvaluator::DERIV_MV_BY_COL,
						       param_indexes);
  EpetraExt::ModelEvaluator::Derivative deriv(dmv);
  outargs.set_DfDp(0, deriv);

  model_->evalModel(inargs, outargs);

  return NOX::Abstract::Group::Ok;
}

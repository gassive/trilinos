// $Id$ 
// $Source$ 

//@HEADER
// ************************************************************************
// 
//            NOX: An Object-Oriented Nonlinear Solver Package
//                 Copyright (2002) Sandia Corporation
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
// Questions? Contact Tammy Kolda (tgkolda@sandia.gov) or Roger Pawlowski
// (rppawlo@sandia.gov), Sandia National Laboratories.
// 
// ************************************************************************
//@HEADER

#include "NOX_Common.H"
#include "NOX_TSF_Group.H"	// class definition
#include "Teuchos_MPIComm.hpp"
#include "SundanceOut.hpp"


using namespace Sundance;

NOX::TSF::Group::Group(const TSFExtended::Vector<double>& initcond, 
  const TSFExtended::NonlinearOperator<double>& nonlinOp,
  const TSFExtended::LinearSolver<double>& linsolver) 
  :
  precision(3),  // 3 digits of accuracy is default
  xVector(initcond, precision, DeepCopy),
  fVector(xVector, precision, ShapeCopy),
  newtonVector(xVector, precision, ShapeCopy),
  gradientVector(xVector, precision, ShapeCopy),
  solver(linsolver),
  jacobian(),
  nonlinearOp(nonlinOp),
  normF(0.0)
{  
  nonlinearOp.setEvalPt(xVector.getTSFVector());
  resetIsValid();
}

NOX::TSF::Group::Group(const TSFExtended::NonlinearOperator<double>& nonlinOp,
  const TSFExtended::LinearSolver<double>& linsolver) 
  :
  precision(3), // 3 digits of accuracy is default
  xVector(nonlinOp.getInitialGuess(), precision, DeepCopy),
  fVector(xVector, precision, ShapeCopy),
  newtonVector(xVector, precision, ShapeCopy),
  gradientVector(xVector, precision, ShapeCopy),
  solver(linsolver),
  jacobian(),
  nonlinearOp(nonlinOp),
  normF(0.0)
{  
  nonlinearOp.setEvalPt(xVector.getTSFVector());
  resetIsValid();
}

NOX::TSF::Group::Group(const TSFExtended::Vector<double>& initcond, 
  const TSFExtended::NonlinearOperator<double>& nonlinOp,
  const TSFExtended::LinearSolver<double>& linsolver,
  int numdigits) 
  :
  precision(numdigits),
  xVector(initcond, precision, DeepCopy),
  fVector(xVector, precision, ShapeCopy),
  newtonVector(xVector, precision, ShapeCopy),
  gradientVector(xVector, precision, ShapeCopy),
  solver(linsolver),
  jacobian(),
  nonlinearOp(nonlinOp),
  normF(0.0)
{  
  nonlinearOp.setEvalPt(xVector.getTSFVector());
  resetIsValid();
}

NOX::TSF::Group::Group(const TSFExtended::NonlinearOperator<double>& nonlinOp,
  const TSFExtended::LinearSolver<double>& linsolver,
  int numdigits) 
  :
  precision(numdigits),
  xVector(nonlinOp.getInitialGuess(), precision, DeepCopy),
  fVector(xVector, precision, ShapeCopy),
  newtonVector(xVector, precision, ShapeCopy),
  gradientVector(xVector, precision, ShapeCopy),
  solver(linsolver),
  jacobian(),
  nonlinearOp(nonlinOp),
  normF(0.0)
{  
  nonlinearOp.setEvalPt(xVector.getTSFVector());
  resetIsValid();
}


NOX::TSF::Group::Group(const NOX::TSF::Group& source, NOX::CopyType type) :
  precision(source.precision),
  xVector(source.xVector, precision, type), 
  fVector(source.fVector, precision, type),  
  newtonVector(source.newtonVector, precision, type),
  gradientVector(source.gradientVector, precision, type),
  solver(source.solver),
  jacobian(source.jacobian),
  nonlinearOp(source.nonlinearOp),
  isValidF(false),
  isValidJacobian(false),
  isValidGradient(false),
  isValidNewton(false),
  normF(0.0)
{
  switch (type) 
  {
    
    case NOX::DeepCopy:
    
      isValidF = source.isValidF;
      isValidGradient = source.isValidGradient;
      isValidNewton = source.isValidNewton;
      isValidJacobian = source.isValidJacobian;
      normF = source.normF;
      break;

    case NOX::ShapeCopy:
      resetIsValid();
      normF = 0.0;
      break;

    default:
      Out::os() << "NOX:TSF::Group - invalid CopyType for copy constructor." << std::endl;
      throw "NOX TSF Error";
  }

}

NOX::TSF::Group::~Group() 
{
}

void NOX::TSF::Group::resetIsValid() //private
{
  isValidF = false;
  isValidJacobian = false;
  isValidGradient = false;
  isValidNewton = false;
}

#ifdef TRILINOS_6
NOX::Abstract::Group* NOX::TSF::Group::clone(NOX::CopyType type) const 
{
  return new NOX::TSF::Group(*this, type);
}
#else
RCP<NOX::Abstract::Group> NOX::TSF::Group::clone(NOX::CopyType type) const 
{
  return rcp(new NOX::TSF::Group(*this, type));
}
#endif

NOX::Abstract::Group& NOX::TSF::Group::operator=(const NOX::Abstract::Group& source)
{
  return operator=(dynamic_cast<const NOX::TSF::Group&> (source));
}

NOX::Abstract::Group& NOX::TSF::Group::operator=(const NOX::TSF::Group& source)
{
  if (this != &source) 
  {

    // Deep Copy of the xVector
    xVector = source.xVector;
    nonlinearOp = source.nonlinearOp;
    solver = source.solver;
    jacobian = source.jacobian;
    precision = source.precision;

    // Update the isValidVectors
    isValidF = source.isValidF;
    isValidGradient = source.isValidGradient;
    isValidNewton = source.isValidNewton;
    isValidJacobian = source.isValidJacobian;
    
    // Only copy vectors that are valid
    if (isValidF) 
    {
      fVector = source.fVector;
      normF = source.normF;
    }

    if (isValidGradient)
      gradientVector = source.gradientVector;
    
    if (isValidNewton)
      newtonVector = source.newtonVector;
    
    if (isValidJacobian)
      jacobian = source.jacobian;
  }
  return *this;
}

void NOX::TSF::Group::setX(const NOX::Abstract::Vector& y) 
{
  setX(dynamic_cast<const NOX::TSF::Vector&> (y));
}

void NOX::TSF::Group::setX(const NOX::TSF::Vector& y) 
{
  resetIsValid();
  nonlinearOp.setEvalPt(y.getTSFVector());
  xVector = y;
}

void NOX::TSF::Group::computeX(const NOX::Abstract::Group& grp, 
  const NOX::Abstract::Vector& d, 
  double step) 
{
  // Cast to appropriate type, then call the "native" computeX
  const Group& tsfgrp = dynamic_cast<const NOX::TSF::Group&> (grp);
  const Vector& tsfd = dynamic_cast<const NOX::TSF::Vector&> (d);
  computeX(tsfgrp, tsfd, step); 
}

void NOX::TSF::Group::computeX(const Group& grp, const Vector& d, double step) 
{
  resetIsValid();
  xVector.update(1.0, grp.xVector, step, d);
}

NOX::Abstract::Group::ReturnType NOX::TSF::Group::computeF() 
{
 
  if (verb() > 2)
  {
    Out::os() << "calling computeF()" << std::endl;
  }

  if (isValidF)
  {
    if (verb() > 2)
    {
      Out::os() << "reusing F" << std::endl;
    }
    return NOX::Abstract::Group::Ok;
  }
  else
  {
    if (verb() > 2)
    {
      Out::os() << "computing new F" << std::endl;
    }
    nonlinearOp.setEvalPt(xVector.getTSFVector());
    fVector = nonlinearOp.getFunctionValue();
    isValidF = true;
    normF = fVector.norm();
  }

  return (isValidF) ? (NOX::Abstract::Group::Ok) : (NOX::Abstract::Group::Failed);
}

NOX::Abstract::Group::ReturnType NOX::TSF::Group::computeJacobian() 
{

  if (verb() > 2)
  {
    Out::os() << "calling computeJ()" << std::endl;
  }

  // Skip if the Jacobian is already valid
  if (isValidJacobian)
  {
    return NOX::Abstract::Group::Ok;
  }
  else
  {
    nonlinearOp.setEvalPt(xVector.getTSFVector());
    jacobian = nonlinearOp.getJacobian();

    isValidJacobian = true;
  }
  return (isValidJacobian) ? (NOX::Abstract::Group::Ok) : (NOX::Abstract::Group::Failed);
}

NOX::Abstract::Group::ReturnType NOX::TSF::Group::computeGradient() 
{
  if (verb() > 2)
  {
    Out::os() << "calling computeGrad()" << std::endl;
  }
  if (isValidGradient)
  {
    return NOX::Abstract::Group::Ok;
  }
  else
  {
    if (!isF()) 
    {
      Out::os() << "ERROR: NOX::TSF::Group::computeGrad() - F is out of date wrt X!" << std::endl;
      return NOX::Abstract::Group::BadDependency;
    }

    if (!isJacobian()) 
    {
      Out::os() << "ERROR: NOX::TSF::Group::computeGrad() - Jacobian is out of date wrt X!" << std::endl;
      return NOX::Abstract::Group::BadDependency;
    }
  
    // Compute Gradient = J' * F

    NOX::Abstract::Group::ReturnType status 
      = applyJacobianTranspose(fVector,gradientVector);
    isValidGradient = (status == NOX::Abstract::Group::Ok);

    // Return result
    return status;
  }
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::computeNewton(Teuchos::ParameterList& p) 
{
  if (isNewton())
  {
    return NOX::Abstract::Group::Ok;
  }
  else
  {
    if (!isF()) 
    {
      Out::os() << "ERROR: NOX::Example::Group::computeNewton() - invalid F" 
           << std::endl;
      throw "NOX Error";
    }

    if (!isJacobian()) 
    {
      Out::os() << "ERROR: NOX::Example::Group::computeNewton() - invalid Jacobian" << std::endl;
      throw "NOX Error";
    }

/*
    if (p.isParameter("Tolerance"))
    {
      double tol = p.get("Tolerance", tol);
      solver.updateTolerance(tol);
    }
*/

    NOX::Abstract::Group::ReturnType status 
      = applyJacobianInverse(p, fVector, newtonVector);
    isValidNewton = (status == NOX::Abstract::Group::Ok);

    
    // Scale soln by -1
    newtonVector.scale(-1.0);

    if (verb() > 0)
    {
      Out::os() << "newton step" << std::endl;
      newtonVector.getTSFVector().print(Out::os());
    }
      
    // Return solution
    return status;
  }
}


NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobian(const Abstract::Vector& input, 
  NOX::Abstract::Vector& result) const
{
  const NOX::TSF::Vector& tsfinput = dynamic_cast<const NOX::TSF::Vector&> (input);
  NOX::TSF::Vector& tsfresult = dynamic_cast<NOX::TSF::Vector&> (result);
  return applyJacobian(tsfinput, tsfresult);
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobian(const NOX::TSF::Vector& input, 
  NOX::TSF::Vector& result) const
{
  // Check validity of the Jacobian
  if (!isJacobian()) 
  {
    return NOX::Abstract::Group::BadDependency;
  }
  else
  {
    // Compute result = J * input
    jacobian.apply(input.getTSFVector(),result.getTSFVector());
    return NOX::Abstract::Group::Ok;
  }
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobianTranspose(const NOX::Abstract::Vector& input, 
  NOX::Abstract::Vector& result) const
{
  const NOX::TSF::Vector& tsfinput = dynamic_cast<const NOX::TSF::Vector&> (input);
  NOX::TSF::Vector& tsfresult = dynamic_cast<NOX::TSF::Vector&> (result);
  return applyJacobianTranspose(tsfinput, tsfresult);
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobianTranspose(const NOX::TSF::Vector& input, NOX::TSF::Vector& result) const
{
  // Check validity of the Jacobian
  if (!isJacobian()) 
  {
    return NOX::Abstract::Group::BadDependency;
  }
  else
  {
    // Compute result = J^T * input
    jacobian.applyTranspose(input.getTSFVector(), result.getTSFVector());
      
    return NOX::Abstract::Group::Ok;
  }
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobianInverse(Teuchos::ParameterList& p, 
  const Abstract::Vector& input, 
  NOX::Abstract::Vector& result) const 
{
  const NOX::TSF::Vector& tsfinput = dynamic_cast<const NOX::TSF::Vector&> (input);
  NOX::TSF::Vector& tsfresult = dynamic_cast<NOX::TSF::Vector&> (result); 
  return applyJacobianInverse(p, tsfinput, tsfresult);
}

NOX::Abstract::Group::ReturnType 
NOX::TSF::Group::applyJacobianInverse(Teuchos::ParameterList& p, 
  const NOX::TSF::Vector& input, 
  NOX::TSF::Vector& result) const 
{

  if (!isJacobian()) 
  {
    Out::os() << "ERROR: NOX::TSF::Group::applyJacobianInverse() - invalid Jacobian" << std::endl;
    throw "NOX Error";

  }
/*
  if (p.isParameter("Tolerance"))
  {
    double tol = p.get("Tolerance", tol);
    solver.updateTolerance(tol);
  }
*/
  if (verb() > 4)
  {
    Out::os() << "---------------- applying J^-1 ------------------" << std::endl;
    Out::os() << "J=" << std::endl;
    jacobian.print(Out::os());
    Out::os() << "F=" << std::endl;
    input.getTSFVector().print(Out::os());
  }

  TSFExtended::SolverState<double> status 
    = solver.solve(jacobian, input.getTSFVector(),
      result.getTSFVector());
  
  if (status.finalState() != TSFExtended::SolveConverged)
  {
    return NOX::Abstract::Group::Failed;
  }
  else
  {
    if (verb() > 2)
    {
      Out::os() << "soln=" << std::endl;
      result.getTSFVector().print(Out::os());
    }
    return NOX::Abstract::Group::Ok;
  }

}

bool NOX::TSF::Group::isF() const 
{   
  return isValidF;
}

bool NOX::TSF::Group::isJacobian() const 
{  
  return isValidJacobian;
}

bool NOX::TSF::Group::isGradient() const 
{   
  return isValidGradient;
}

bool NOX::TSF::Group::isNewton() const 
{   
  return isValidNewton;
}

const NOX::Abstract::Vector& NOX::TSF::Group::getX() const 
{
  return xVector;
}

const NOX::Abstract::Vector& NOX::TSF::Group::getF() const 
{  
  if (verb() > 2)
  {
    Out::os() << "calling getF()" << std::endl;
  }
  TEST_FOR_EXCEPTION(!isF(), runtime_error, 
    "calling getF() with invalid function value");
  return fVector;
}

double NOX::TSF::Group::getNormF() const
{
  if (verb() > 2)
  {
    Out::os() << "normF = " << normF << std::endl;
  }
  TEST_FOR_EXCEPTION(!isF(), runtime_error, 
    "calling normF() with invalid function value");
  return normF;
}

const NOX::Abstract::Vector& NOX::TSF::Group::getGradient() const 
{ 
  return gradientVector;
}

const NOX::Abstract::Vector& NOX::TSF::Group::getNewton() const 
{
  return newtonVector;
}

void NOX::TSF::Group::print() const
{
  cout << "x = " << xVector << "\n";

  if (isValidF) {
    cout << "F(x) = " << fVector << "\n";
    cout << "|| F(x) || = " << normF << "\n";
  }
  else
    cout << "F(x) has not been computed" << "\n";
  
  cout << std::endl;
}




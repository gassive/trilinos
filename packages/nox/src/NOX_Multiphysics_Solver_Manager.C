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

#include "NOX_Multiphysics_Solver_Manager.H"	// class definition
#include "NOX_Multiphysics_DataExchange_Interface.H"
#include "NOX_Multiphysics_Solver_FixedPointBased.H"
#include "NOX_Utils.H"
#include "Teuchos_RCP.hpp"


NOX::Multiphysics::Solver::Manager::Manager(
        const Teuchos::RCP<vector<Teuchos::RCP<NOX::Solver::Generic> > >& solvers, 
        const Teuchos::RCP<NOX::Multiphysics::DataExchange::Interface>& i, 
	const Teuchos::RCP<NOX::StatusTest::Generic>& t, 
	const Teuchos::RCP<Teuchos::ParameterList>& p) :
  utils(p->sublist("Printing")),
  method(""),
  cplPtr(NULL)
{
  reset(solvers, i, t, p);
}

NOX::Multiphysics::Solver::Manager::Manager(
        const Teuchos::RCP<NOX::Abstract::Group>& grp, 
	const Teuchos::RCP<NOX::StatusTest::Generic>& t, 
	const Teuchos::RCP<Teuchos::ParameterList>& p) :
  utils(p->sublist("Printing")),
  method(""),
  cplPtr(NULL)
{
  
}

NOX::Multiphysics::Solver::Manager::Manager() :
  method(""),
  cplPtr(NULL)
{
}

NOX::Multiphysics::Solver::Manager::~Manager()
{
  delete cplPtr;
}

bool NOX::Multiphysics::Solver::Manager::reset(
      const Teuchos::RCP<vector<Teuchos::RCP<NOX::Solver::Generic> > >& solvers, 
      const Teuchos::RCP<NOX::Multiphysics::DataExchange::Interface>& interface, 
      const Teuchos::RCP<NOX::StatusTest::Generic>& tests, 
      const Teuchos::RCP<Teuchos::ParameterList>& params)
{
  string newmethod = 
    params->get("Coupling Strategy", "Fixed Point Based");

  if ((method == newmethod) && (cplPtr != NULL))
  {
    return cplPtr->reset(solvers, interface, tests, params);
  }
  else 
  {
    method = newmethod;

    delete cplPtr;
    cplPtr = NULL;
    
    if( method == "Fixed Point Based" ) 
    {	
      cplPtr = new NOX::Multiphysics::Solver::FixedPointBased(solvers, interface, tests, params);
    } 
    else 
    {
      utils.out() << "ERROR: NOX::Multiphysics::Solver::Manager::reset - Invalid solver choice " << method << endl;
      throw "NOX Error";
    }

    if (cplPtr == NULL) 
    {
      utils.err() << "NOX::Multiphysics::Solver::Manager::reset - Null pointer error" << endl;
      return false;
    }

    return true;
  }
}

void
NOX::Multiphysics::Solver::Manager::reset(
      const NOX::Abstract::Vector& initialGuess, 
      const Teuchos::RCP<NOX::StatusTest::Generic>& tests)
{
  cplPtr->reset(initialGuess, tests);
}

void 
NOX::Multiphysics::Solver::Manager::
reset(const Abstract::Vector& initialGuess)
{
  cplPtr->reset(initialGuess);
}

// PRIVATE
void NOX::Multiphysics::Solver::Manager::deprecated(const string& oldName, const string& newName) const
{
  utils.out() << "Warning: NOX::Multiphysics::Solver::Manager::reset - " 
       << "Nonlinear Solver choice \"" << oldName << "\" is deprecated.\n"
       << "                                       " 
       << "Use \"" << newName << "\" instead." 
       << endl;
}

NOX::StatusTest::StatusType NOX::Multiphysics::Solver::Manager::getStatus()
{
  checkNullPtr("getStatus");
  return cplPtr->getStatus();
}

NOX::StatusTest::StatusType NOX::Multiphysics::Solver::Manager::step()
{
  checkNullPtr("step");
  return cplPtr->step();
}

NOX::StatusTest::StatusType NOX::Multiphysics::Solver::Manager::solve()
{
  checkNullPtr("solve");
  return cplPtr->solve();
}

const NOX::Abstract::Group& NOX::Multiphysics::Solver::Manager::getSolutionGroup() const
{
  checkNullPtr("getSolutionGroup");
  return cplPtr->getSolutionGroup();
}

const NOX::Abstract::Group& NOX::Multiphysics::Solver::Manager::getPreviousSolutionGroup() const
{
  checkNullPtr("getPreviousSolutionGroup");
  return cplPtr->getPreviousSolutionGroup();
}

int NOX::Multiphysics::Solver::Manager::getNumIterations() const
{
  if (cplPtr == NULL)
    return 0;

  return cplPtr->getNumIterations();
}

const Teuchos::ParameterList& NOX::Multiphysics::Solver::Manager::getList() const
{
  checkNullPtr("getList");
  return cplPtr->getList();
}

// PRIVATE
void NOX::Multiphysics::Solver::Manager::checkNullPtr(const string& fname) const
{
  if (cplPtr == NULL) 
  {
    utils.out() << "NOX::Multiphysics::Solver::Manager::" << fname << " - Null pointer error" << endl;
    throw "NOX Error";
  }
}


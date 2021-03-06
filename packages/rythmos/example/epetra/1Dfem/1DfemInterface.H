//@HEADER
// ***********************************************************************
//
//                     Rythmos Package
//                 Copyright (2006) Sandia Corporation
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
// Questions? Contact Todd S. Coffey (tscoffe@sandia.gov)
//
// ***********************************************************************
//@HEADER
                                                                                
// Finite Element Problem Class
/* Provides function (F) and Jacobian evaluations for the following equation 
 * via a 1D linear finite element discretization with Epetra objects.
 *
 * d2u 
 * --- - k * u**2 = 0
 * dx2
 *
 * subject to @ x=0, u=1
 */

#ifndef TEST_EPETRA_1DFEM_INTERFACE_EPETRANEW_H
#define TEST_EPETRA_1DFEM_INTERFACE_EPETRANEW_H

#include "NOX_Epetra_Interface_Required.H" // base class
#include "NOX_Epetra_Interface_Jacobian.H" // base class
#include "NOX_Epetra_Interface_Preconditioner.H" // base class

// Forward Declarations
class Epetra_Comm;
class Epetra_Map; 
class Epetra_Vector;
class Epetra_Import;
class Epetra_Operator;
class Epetra_CrsGraph;
class Epetra_RowMatrix;
class Epetra_CrsMatrix;
namespace NOX {
  namespace Parameter {
    class List;
  }
}

// Finite Element Problem Class
class Interface : public NOX::Epetra::Interface::Required,
		  public NOX::Epetra::Interface::Jacobian,
		  public NOX::Epetra::Interface::Preconditioner { 

public:

  // Constructor
  Interface(int NumGlobalElements, Epetra_Comm& Comm, double xmin = 0.0,
            double xmax = 1.0 );
  
  // Destructor
  virtual ~Interface();

  virtual bool computeF(const Epetra_Vector& x, Epetra_Vector& FVec, 
		FillType fillType = Residual);
  virtual bool computeJacobian(const Epetra_Vector& x, Epetra_Operator& Jac);
  virtual bool computePrecMatrix(const Epetra_Vector& x);
  virtual bool computePreconditioner(const Epetra_Vector& x,
				     Epetra_Operator& Prec,
				     Teuchos::ParameterList* precParams = 0);


  // Return a reference to the Epetra_Vector with the initial guess
  // that is generated by the Interface class.
  Epetra_Vector& getSolution();
  
  // Return a reference to the Epetra_Vector holding the mesh data
  Epetra_Vector& getMesh();
  
  // Return a reference to the Epetra_CrsMatrix with the Jacobian
  // that is generated by the Interface class.
  Epetra_CrsMatrix& getJacobian();

  // Set PDE factor
  bool setPDEfactor(double value) { factor = value; return true; };

  // Return a reference to the Epetra_Map used for the Solution Epetra_Vector
  Epetra_Map& getMap();
  
  // Return a reference to the Epetra_Graph used for the Jacobian
  Epetra_CrsGraph& getGraph();

protected:

  // Evaluates the function (F) and/or the Jacobian using the solution 
  // values in solnVector.
  virtual bool evaluate(FillType f, const Epetra_Vector *solnVector, 
		Epetra_Vector *rhsVector, Epetra_RowMatrix *matrix);

  // Inserts the global column indices into the Graph
  virtual bool createGraph();

  // Set initial condition for solution vector
  virtual bool initializeSoln();

protected:

  enum TargetObjectType {FUNCTION, JACOBIAN};

  int NumGlobalElements;  // Total Number of elements
  int NumMyElements;      // Number of elements owned by this process
  int MyPID;              // Process number
  int NumProc;            // Total number of processes

  double xmin;
  double xmax;
  double factor; 	  // PDE factor

  Epetra_Comm* Comm;
  Epetra_Map* StandardMap; 
  Epetra_Map* OverlapMap;
  Epetra_Import* Importer;
  Epetra_Vector* initialSolution;
  Epetra_Vector* rhs;
  Epetra_CrsGraph* Graph;
  Epetra_CrsMatrix* jacobian;
  Epetra_Vector* xptr;

};

//==================================================================
// Finite Element Basis Object
class Basis {

 public:
  // Constructor
  Basis();

  // Destructor
  ~Basis();

  // Calculates the values of u and x at the specified gauss point
  void computeBasis(int gp, double *x, double *u, double *uold = 0);

 public:
  // Variables that are calculated at the gauss point
  double *phi, *dphide; 
  double uu, xx, duu, eta, wt;
  double dx;
  // These are only needed for transient
  double uuold, duuold;
};

#endif






// @HEADER
// ***********************************************************************
// 
//                      Didasko Tutorial Package
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
// Questions about Didasko? Contact Marzio Sala (marzio.sala _AT_ gmail.com)
// 
// ***********************************************************************
// @HEADER

// Trilinos Tutorial
// -----------------
// Simple nonlinear PDE problem.
// This file shows how to solve the nonlinear problem
//
// -\Delta u + \lambda e^u = 0  in \Omega = (0,1) \times (0,1)
//                       u = 0  on \partial \Omega
//
// using NOX 

#include "Didasko_ConfigDefs.h"
#if defined(HAVE_DIDASKO_EPETRA) && defined(HAVE_DIDASKO_NOX) && defined(HAVE_NOX_EPETRA)

#include "Epetra_ConfigDefs.h"
#ifdef HAVE_MPI
#include "mpi.h"
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_CrsMatrix.h"
#include "NOX.H"
#include "NOX_Epetra_Interface_Required.H"
#include "NOX_Epetra_Interface_Jacobian.H"
#include "NOX_Epetra_LinearSystem_AztecOO.H"
#include "NOX_Epetra_Group.H"

// this is required to know the number of lower, upper, left and right
// node for each node of the Cartesian grid (composed by nx \timex ny 
// elements)

static void  get_neighbours( const int i, const int nx, const int ny,
			     int & left, int & right, 
			     int & lower, int & upper) 
{

  int ix, iy;
  ix = i%nx;
  iy = (i - ix)/nx;

  if( ix == 0 ) 
    left = -1;
  else 
    left = i-1;
  if( ix == nx-1 ) 
    right = -1;
  else
    right = i+1;
  if( iy == 0 ) 
    lower = -1;
  else
    lower = i-nx;
  if( iy == ny-1 ) 
    upper = -1;
  else
    upper = i+nx;

  return;

}

// This function creates a CrsMatrix, whose elements corresponds
// to the discretization of a Laplacian over a Cartesian grid,
// with nx grid point along the x-axis and and ny grid points 
// along the y-axis. For the sake of simplicity, I suppose that
// all the nodes in the matrix are internal nodes (Dirichlet
// boundary nodes are supposed to have been already condensated)

Epetra_CrsMatrix * CreateLaplacian( const int nx, const int ny,
				    const Epetra_Comm * Comm)
{

  int NumGlobalElements = nx * ny;
    
  // create a map
  Epetra_Map * Map = new Epetra_Map(NumGlobalElements,0,*Comm);
  // local number of rows
  int NumMyElements = Map->NumMyElements();
  // get update list
  int * MyGlobalElements = Map->MyGlobalElements();

  double hx = 1.0/(nx-1);
  double hy = 1.0/(ny-1);
  double off_left  = -1.0/(hx*hx);
  double off_right = -1.0/(hx*hx);
  double off_lower = -1.0/(hy*hy);
  double off_upper = -1.0/(hy*hy);
  double diag      =  2.0/(hx*hx) + 2.0/(hy*hy);
  
  int left, right, lower, upper;
    
  // a bit overestimated the nonzero per row
  
  Epetra_CrsMatrix * A = new Epetra_CrsMatrix(Copy,*Map,5);
    
  // Add  rows one-at-a-time
    
  double *Values = new double[4];
  int *Indices = new int[4];
    
  for( int i=0 ; i<NumMyElements; ++i ) {
    int NumEntries=0;
    get_neighbours(  MyGlobalElements[i], nx, ny, 
		     left, right, lower, upper);
    if( left != -1 ) {
      Indices[NumEntries] = left;
      Values[NumEntries] = off_left;
      ++NumEntries;
    }
    if( right != -1 ) {
      Indices[NumEntries] = right;
      Values[NumEntries] = off_right;
      ++NumEntries;
    }
    if( lower != -1 ) {
      Indices[NumEntries] = lower;
      Values[NumEntries] = off_lower;
      ++NumEntries;
    }
    if( upper != -1 ) {
      Indices[NumEntries] = upper;
      Values[NumEntries] = off_upper;
      ++NumEntries;
    }
    // put the off-diagonal entries
    A->InsertGlobalValues(MyGlobalElements[i], NumEntries, Values, Indices);
    // Put in the diagonal entry
    A->InsertGlobalValues(MyGlobalElements[i], 1, &diag, MyGlobalElements+i);
  }

  // put matrix in local ordering
  A->FillComplete();

  delete [] Indices;
  delete [] Values;
  delete    Map;

  return A;
  
} /* createJacobian */

// ==========================================================================
// This class contians the main definition of the nonlinear problem at
// hand. A method is provided to compute F(x) for a given x, and another
// method to update the entries of the Jacobian matrix, for a given x.
// As the Jacobian matrix J can be written as
//    J = L + diag(lambda*exp(x[i])),
// where L corresponds to the discretization of a Laplacian, and diag
// is a diagonal matrix with lambda*exp(x[i]). Basically, to update
// the jacobian we simply update the diagonal entries. Similarly, to compute
// F(x), we reset J to be equal to L, then we multiply it by the
// (distributed) vector x, then we add the diagonal contribution
// ==========================================================================

class PDEProblem {

public:

  // constructor. Requires the number of nodes along the x-axis
  // and y-axis, the value of lambda, and the Epetra_Communicator
  // (to define a Map, which is a linear map in this case)
  PDEProblem(const int nx, const int ny, const double lambda,
	     const Epetra_Comm * Comm) :
    nx_(nx), ny_(ny), lambda_(lambda) 
  {
    hx_ = 1.0/(nx_-1);
    hy_ = 1.0/(ny_-1);
    Matrix_ = CreateLaplacian(nx_,ny_,Comm);
  }

  // destructor
  ~PDEProblem() 
  {
    delete Matrix_;
  }

  // compute F(x)
  void ComputeF(const Epetra_Vector & x, Epetra_Vector & f) 
  {
    // reset diagonal entries
    double diag      =  2.0/(hx_*hx_) + 2.0/(hy_*hy_);
  
    int NumMyElements = Matrix_->Map().NumMyElements();
    // get update list
    int * MyGlobalElements = Matrix_->Map().MyGlobalElements( );
    
    for( int i=0 ; i<NumMyElements; ++i ) {
      // Put in the diagonal entry
      Matrix_->ReplaceGlobalValues(MyGlobalElements[i], 1, &diag, MyGlobalElements+i);
    }
    // matrix-vector product (intra-processes communication occurs
    // in this call)
    Matrix_->Multiply(false,x,f);

    // add diagonal contributions
    for( int i=0 ; i<NumMyElements; ++i ) {
      // Put in the diagonal entry

      f[i] += lambda_*exp(x[i]);   
    }
  }

  // update the Jacobian matrix for a given x
  void UpdateJacobian(const Epetra_Vector & x) 
  {
    
    double diag      =  2.0/(hx_*hx_) + 2.0/(hy_*hy_);
  
    int NumMyElements = Matrix_->Map().NumMyElements();
    // get update list
    int * MyGlobalElements = Matrix_->Map().MyGlobalElements( );
  
    for( int i=0 ; i<NumMyElements; ++i ) {
      // Put in the diagonal entry

      double newdiag = diag + lambda_*exp(x[i]); 

      Matrix_->ReplaceGlobalValues(MyGlobalElements[i], 1, 
				   &newdiag, MyGlobalElements+i);
    }

  }

  // returns a pointer to the internally stored matrix
  Epetra_CrsMatrix * GetMatrix()
  {
    return Matrix_;
  }
  
private:
  
  int nx_, ny_;
  double hx_, hy_;
  Epetra_CrsMatrix * Matrix_;
  double lambda_;

}; /* class PDEProblem */
 
// ==========================================================================
// This is the main NOX class for this example. Here we define
// the interface between the nonlinear problem at hand, and NOX.
// The constructor accepts a PDEProblem object. Using a pointer
// to this object, we can update the Jacobian and compute F(x),
// using the definition of our problem. This interface is bit
// crude: For instance, no PrecMatrix nor Preconditioner is specified.
// ==========================================================================

class SimpleProblemInterface : public NOX::Epetra::Interface::Required,
                               public NOX::Epetra::Interface::Jacobian
{

public:
 
  //! Constructor
  SimpleProblemInterface( PDEProblem * Problem ) :
    Problem_(Problem) {};

  //! Destructor
  ~SimpleProblemInterface() 
  {
  };

  bool computeF(const Epetra_Vector & x, Epetra_Vector & f,
                NOX::Epetra::Interface::Required::FillType F )
  {
    Problem_->ComputeF(x,f);
    return true;
  };
  
  bool computeJacobian(const Epetra_Vector & x, Epetra_Operator & Jac)
  {

    Problem_->UpdateJacobian(x);
    return true;
  }

  bool computePrecMatrix(const Epetra_Vector & x, Epetra_RowMatrix & M) 
  {
    cout << "*ERR* SimpleProblem::preconditionVector()\n";
    cout << "*ERR* don't use explicit preconditioning" << endl;
    throw 1;
  }  
  
  bool computePreconditioner(const Epetra_Vector & x, Epetra_Operator & O)
  {
    cout << "*ERR* SimpleProblem::preconditionVector()\n";
    cout << "*ERR* don't use explicit preconditioning" << endl;
    throw 1;
  }  

private:
  
  PDEProblem * Problem_;
  
}; /* class SimpleProblemInterface */

// =========== //
// main driver //
// =========== //

int main( int argc, char **argv )
{

#ifdef HAVE_MPI
  MPI_Init(&argc, &argv);
  Epetra_MpiComm Comm(MPI_COMM_WORLD);
#else
  Epetra_SerialComm Comm;
#endif

  // define the parameters of the nonlinear PDE problem
  int nx = 5;
  int ny = 6;  
  double lambda = 1.0;

  PDEProblem Problem(nx,ny,lambda,&Comm);
 
  // starting solution, here a zero vector
  Epetra_Vector InitialGuess(Problem.GetMatrix()->Map());
  InitialGuess.PutScalar(0.0);

  // Set up the problem interface
  Teuchos::RCP<SimpleProblemInterface> interface = 
    Teuchos::rcp(new SimpleProblemInterface(&Problem) );
  
  // Create the top level parameter list
  Teuchos::RCP<Teuchos::ParameterList> nlParamsPtr =
    Teuchos::rcp(new Teuchos::ParameterList);
  Teuchos::ParameterList& nlParams = *(nlParamsPtr.get());

  // Set the nonlinear solver method
  nlParams.set("Nonlinear Solver", "Line Search Based");

  // Set the printing parameters in the "Printing" sublist
  Teuchos::ParameterList& printParams = nlParams.sublist("Printing");
  printParams.set("MyPID", Comm.MyPID()); 
  printParams.set("Output Precision", 3);
  printParams.set("Output Processor", 0);
  printParams.set("Output Information", 
			NOX::Utils::OuterIteration + 
			NOX::Utils::OuterIterationStatusTest + 
			NOX::Utils::InnerIteration +
			NOX::Utils::Parameters + 
			NOX::Utils::Details + 
			NOX::Utils::Warning);

  // start definition of nonlinear solver parameters
  // Sublist for line search 
  Teuchos::ParameterList& searchParams = nlParams.sublist("Line Search");
  searchParams.set("Method","More'-Thuente");

  // Sublist for direction
  Teuchos::ParameterList& dirParams = nlParams.sublist("Direction");
  dirParams.set("Method", "Newton");

  Teuchos::ParameterList& newtonParams = dirParams.sublist("Newton");
  newtonParams.set("Forcing Term Method", "Constant");

  // Sublist for linear solver for the Newton method
  Teuchos::ParameterList& lsParams = newtonParams.sublist("Linear Solver");
  lsParams.set("Aztec Solver", "GMRES");  
  lsParams.set("Max Iterations", 800);  
  lsParams.set("Tolerance", 1e-4);
  lsParams.set("Output Frequency", 50);    
  lsParams.set("Aztec Preconditioner", "ilu"); 

  Teuchos::RCP<Epetra_CrsMatrix> A = Teuchos::rcp( Problem.GetMatrix(), false );

  Teuchos::RCP<NOX::Epetra::Interface::Required> iReq = interface;
  Teuchos::RCP<NOX::Epetra::Interface::Jacobian> iJac = interface;
  Teuchos::RCP<NOX::Epetra::LinearSystemAztecOO> linSys = 
    Teuchos::rcp(new NOX::Epetra::LinearSystemAztecOO(printParams, lsParams,
						      iReq,
						      iJac, A, 
						      InitialGuess));

  // Need a NOX::Epetra::Vector for constructor
  NOX::Epetra::Vector noxInitGuess(InitialGuess, NOX::DeepCopy);
  Teuchos::RCP<NOX::Epetra::Group> grpPtr = 
    Teuchos::rcp(new NOX::Epetra::Group(printParams, 
					iReq, 
					noxInitGuess, 
					linSys)); 

  // Set up the status tests
  Teuchos::RCP<NOX::StatusTest::NormF> testNormF = 
    Teuchos::rcp(new NOX::StatusTest::NormF(1.0e-4));
  Teuchos::RCP<NOX::StatusTest::MaxIters> testMaxIters = 
    Teuchos::rcp(new NOX::StatusTest::MaxIters(20));
  // this will be the convergence test to be used
  Teuchos::RCP<NOX::StatusTest::Combo> combo = 
    Teuchos::rcp(new NOX::StatusTest::Combo(NOX::StatusTest::Combo::OR, 
					    testNormF, testMaxIters));

  // Create the solver
  Teuchos::RCP<NOX::Solver::Generic> solver = 
    NOX::Solver::buildSolver(grpPtr, combo, nlParamsPtr);

  // Solve the nonlinear system
  NOX::StatusTest::StatusType status = solver->solve();

if( Comm.MyPID() == 0 )   
  if( NOX::StatusTest::Converged  == status )
    cout << "\n" << "-- NOX solver converged --" << "\n";
  else
    cout << "\n" << "-- NOX solver did not converge --" << "\n";
 
  // Print the answer
  if( Comm.MyPID() == 0 ) {
  cout << "\n" << "-- Parameter List From Solver --" << "\n";
  solver->getList().print(cout);
  }

  // Get the Epetra_Vector with the final solution from the solver
  const NOX::Epetra::Group & finalGroup = 
    dynamic_cast<const NOX::Epetra::Group&>(solver->getSolutionGroup());
  const Epetra_Vector & finalSolution = 
      (dynamic_cast<const NOX::Epetra::Vector&>(finalGroup.getX())).getEpetraVector();

  if( Comm.MyPID() == 0 ) cout << "Computed solution : " << endl;
  cout << finalSolution;

#ifdef HAVE_MPI
  MPI_Finalize();
#endif
  return(EXIT_SUCCESS);
} /* main */

#else

#include <stdlib.h>
#include <stdio.h>
#ifdef HAVE_MPI
#include "mpi.h"
#endif

int main(int argc, char *argv[])
{
#ifdef HAVE_MPI
  MPI_Init(&argc,&argv);
#endif
  puts("Please configure Didasko with:");
  puts("--enable-epetra");
  puts("--enable-ifpack");
  puts("--enable-aztecoo");
  puts("--enable-nox-epetra");
  puts("--enable-nox\n");

#ifdef HAVE_MPI
  MPI_Finalize();
#endif
  return(EXIT_SUCCESS);
}

#endif

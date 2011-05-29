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

// Output a vector in MATLAB format

#include "Didasko_ConfigDefs.h"
#if defined(HAVE_DIDASKO_EPETRA)

#include "Epetra_ConfigDefs.h"
#ifdef HAVE_MPI
#include "mpi.h"
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"

/* ======== ============= *
 * function Vector2MATLAB *
 * ======== ============= *
 *
 * Print out a Epetra_Vector in a MATLAB format. Each processor prints out
 * its part, starting from proc 0 to proc NumProc-1. The first line of
 * each processor's output states the number of local rows and of
 * local nonzero elements. Output is finished by "End of Vector Output".
 *
 * Return code:        true if vector has been printed out
 * -----------         false otherwise
 *
 * Parameters:
 * ----------
 *
 * - Epetra_CrsMatrix  reference to vector
 * - Epetra_Map        reference to map 
 */

bool Vector2MATLAB( const Epetra_Vector & v,
		    const Epetra_Map & Map)
{
  
  int MyPID = Map.Comm().MyPID(); 
  int NumProc = Map.Comm().NumProc();
  int MyLength = v.MyLength();
  int GlobalLength = v.GlobalLength();
  
  // print out on cout if no filename is provided

  // write on file the dimension of the matrix

  if( MyPID == 0 ) cout << "v = zeros(" << GlobalLength << ")\n";

  // get update list
  int * MyGlobalElements = Map.MyGlobalElements( );
  
  int Row;

  for( int Proc=0 ; Proc<NumProc ; ++Proc ) {

    if( MyPID == Proc ) {

      cout << "% On proc " << Proc << ": ";
      cout << MyLength << " rows of ";
      cout << GlobalLength << " elements\n";

      for( Row=0 ; Row<MyLength ; ++Row ) {
	cout << "b(" << MyGlobalElements[Row]
	     << ") = " << v[Row] << ";\n";
      }
      
      if( MyPID == NumProc-1  ) {
	cout << "% End of vector\n";
      }
      
    }
      
    Map.Comm().Barrier();
  }

  return true;

} /* Vector2MATLAB */


// =========== //
// main driver //
// =========== //

int main(int argc, char *argv[])
{

#ifdef HAVE_MPI
  MPI_Init(&argc, &argv);
  Epetra_MpiComm Comm(MPI_COMM_WORLD);
#else
  Epetra_SerialComm Comm;
#endif

  // set global dimension to 5, could be any number
  int NumGlobalElements = 5;

  // create a map
  Epetra_Map Map(NumGlobalElements,0,Comm);
  Epetra_Vector A(Map);

  // fill the vector with random elements
  A.Random();

  // and output it to stdout
  Vector2MATLAB( A, Map );

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return( EXIT_SUCCESS );

} /* main */

#else

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
  puts("Please configure Didasko with:\n"
       "--enable-epetra");

  return 0;
}
#endif


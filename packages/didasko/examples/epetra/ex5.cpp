
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

// Basic operations on vectors. Work on the element vectors.

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
#include "Epetra_IntSerialDenseVector.h"

int main(int argc, char *argv[])
{
  
#ifdef HAVE_MPI
  MPI_Init(&argc, &argv);
  Epetra_MpiComm Comm(MPI_COMM_WORLD);
#else
  Epetra_SerialComm Comm;
#endif  

  // Total number of elements in the vector
  int NumElements = 5;

  // Construct a Map with NumElements and index base of 0
  Epetra_Map Map(NumElements, 0, Comm);

  // Create x random vector
  Epetra_Vector x(Map);
 
  // get the local size of the vector
  int MyLength = x.MyLength();

  /* First way to define the vector:   */
  /* use the [] operator on the object */

  for( int i=0 ; i<MyLength ; ++i ) x[i] = 1.0*i;

  /* second way: get a copy of the values, modify them, then copy */
  /* back the result into the vector                              */

  // get vector value, put them in `x_values'
  double *x_values;
  x_values = new double[MyLength];

  x.ExtractCopy( x_values );
  for( int i=0 ; i<MyLength ; ++i ) 
    cout << "extracted value[" << i << "] = " << x_values[i] << endl;

  int* Indices; Indices = new int[MyLength];
  
 // now, modify these values
  for( int i=0 ; i<MyLength ; ++i ) {
    x_values[i] *= 10;
    Indices[i] = i;
  }
  
  // note that the modified values do not affect the internal storage
  // of x. 
  cout << x;

  x.ReplaceMyValues(MyLength, 0, x_values, Indices);

  cout << x;

  // free memory and return
  delete[] x_values;
  delete[] Indices;

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return(0);
  
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

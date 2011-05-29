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

// Generate a matrix using triutils and redistribute with Zoltan.

#include "Didasko_config.h"
#if defined(HAVE_DIDASKO_EPETRA) && defined(HAVE_DIDASKO_EPETRAEXT) 

#include "Epetra_ConfigDefs.h"
#ifdef HAVE_MPI
#include "mpi.h"
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_CrsMatrix.h"
#include "EpetraExt_BlockMapOut.h"
#include "EpetraExt_VectorOut.h"
#include "EpetraExt_RowMatrixOut.h"

int main(int argc, char *argv[]) 
{
#ifdef HAVE_MPI
  MPI_Init(&argc,&argv);
  Epetra_MpiComm Comm (MPI_COMM_WORLD);
#else
  Epetra_SerialComm Comm;
#endif

  // creates few simple objects
  int NumMyElements = 5 * Comm.NumProc();
  Epetra_Map Map(-1, NumMyElements, 0, Comm);
  Epetra_Vector X(Map);
  X.Random();
  Epetra_CrsMatrix A(Copy, Map, 0);

  // create a simple diagonal matrix
  for (int i = 0 ; i < NumMyElements ; ++i)
  {
    int j = Map.GID(i);
    double value = 1.0;
    EPETRA_CHK_ERR(A.InsertGlobalValues(j, 1, &value, &j));
  }
  A.FillComplete();

  EpetraExt::BlockMapToMatrixMarketFile("Map.mm", Map, "test map", "This is a test map");
  EpetraExt::VectorToMatrixMarketFile("X.mm", X, "test vector", "This is a test vector");
  EpetraExt::RowMatrixToMatrixMarketFile("A.mm", A, "test matrix", "This is a test matrix");

  // to read the output in MATLAB:
  // 1) download the mmread.m file from the Matrix Market web site
  // 2) use A = mmread('A.mm')

  // one can also read from Matrix Market
  
#ifdef HAVE_MPI
  MPI_Finalize() ;
#endif

  return(0);
}

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

  puts("Please configure Didasko with:\n"
       "--enable-epetra\n"
       "--enable-epetraext\n");

#ifdef HAVE_MPI
  MPI_Finalize();
#endif
  return 0;
}

#endif


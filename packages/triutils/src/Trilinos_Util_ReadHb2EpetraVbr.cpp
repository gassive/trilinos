// @HEADER
// ***********************************************************************
// 
//                 TriUtils: Trilinos Utilities Package
//                 Copyright (2001) Sandia Corporation
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

#include "Trilinos_Util.h"
#include "iohb.h"
#include "Epetra_Comm.h"
#include "Epetra_BlockMap.h"
#include "Epetra_Vector.h"
#include "Epetra_VbrMatrix.h"

void Trilinos_Util_ReadHb2EpetraVbr(char *data_file, char * partitioning,
				 const Epetra_Comm  &comm, 
				 Epetra_BlockMap *& map, 
				 Epetra_VbrMatrix *& A, 
				 Epetra_Vector *& x, 
				 Epetra_Vector *& b,
				 Epetra_Vector *&xexact) {

  /* Read matrix file and distribute among processors.  
     Returns with this processor's set of rows */ 

  int NumGlobalEquations = 0, NumMyNonzeros = 0;
  double *val_msr = 0, *x_in = 0, *b_in = 0, *xexact_in = 0;
  int *bindx_msr = 0;
  
  /* Set exact solution to NULL */
  xexact = NULL;
  Trilinos_Util_read_hb(data_file, comm.MyPID(), &NumGlobalEquations, &NumMyNonzeros,
			&val_msr,  &bindx_msr, &x_in, &b_in, &xexact_in);
  
  double *val = 0;
  int NumGlobalElements = 0;
  int *indx = 0, *rpntr = 0, *cpntr = 0, *bpntr = 0, *bindx = 0;
  int NumMyBlockEntries = 0, NumMyElements = 0, * MyGlobalElements = 0;
  
  Trilinos_Util_create_vbr(comm, partitioning,
			   &NumGlobalEquations, &NumGlobalElements, 
			   &NumMyNonzeros, &NumMyBlockEntries,
			   &NumMyElements, &MyGlobalElements,
			   bindx_msr, val_msr,
			   &val, &indx, &rpntr, &cpntr,
			   &bpntr, &bindx);
  
  if(comm.MyPID()==0)
    {
      free ((void *) val_msr);
      free ((void *) bindx_msr);
      free ((void *) cpntr);
    }
  
  int * ElementSizeList = 0;
  if (NumMyElements>0) ElementSizeList = new int[NumMyElements];
  
  for (int i=0; i<NumMyElements; i++) ElementSizeList[i] = rpntr[i+1] - rpntr[i];
  
  map = new Epetra_BlockMap(-1, NumMyElements, MyGlobalElements, 
			    ElementSizeList, 0, comm);
  
   A = new Epetra_VbrMatrix(Copy, *map, 0);
  
  /* Add block rows one-at-a-time */
  
  {for (int i=0; i<NumMyElements; i++) {
    int BlockRow = MyGlobalElements[i];
    int NumBlockEntries = bpntr[i+1] - bpntr[i];
    int *BlockIndices = bindx + bpntr[i];
    int ierr = A->BeginInsertGlobalValues(BlockRow, NumBlockEntries, BlockIndices);
    if (ierr!=0) {
      cerr << "Error in BeginInsertGlobalValues(GlobalBlockRow = " << BlockRow 
	   << ") = " << ierr << endl; 
      abort();
    }
    int LDA = ElementSizeList[i];
    int NumRows = LDA;
    for (int j=bpntr[i]; j<bpntr[i+1]; j++) {
      int NumCols = (indx[j+1] - indx[j])/LDA;
      double * Values = val + indx[j];
      ierr = A->SubmitBlockEntry(Values, LDA, NumRows, NumCols);
      if (ierr!=0) {
	cerr << "Error in SubmitBlockEntry, GlobalBlockRow = " << BlockRow 
	     << "GlobalBlockCol = " << BlockIndices[j] << "Error = " << ierr << endl; 
	abort();
      }
    }
    ierr = A->EndSubmitEntries();
    if (ierr!=0) {
      cerr << "Error in EndSubmitEntries(GlobalBlockRow = " << BlockRow 
	   << ") = " << ierr << endl; 
      abort();
    }
  }}
  int ierr=A->FillComplete();    
  if (ierr!=0) cerr << "Error in Epetra_VbrMatrix FillComplete ierr = " << ierr << endl;
  
  xexact = new Epetra_Vector(Copy, *map, xexact_in);
  x = new Epetra_Vector(Copy, *map, x_in);
  b = new Epetra_Vector(Copy, *map, b_in);

  if(comm.MyPID()==0)
    {
      free ((void *) val);
      free ((void *) indx);
      free ((void *) rpntr);
      free ((void *) bpntr);
      free ((void *) bindx);
      free ((void *) b_in);
      free ((void *) x_in);
      free ((void *) xexact_in);
      free ((void *) MyGlobalElements);
      delete [] ElementSizeList;
    }
  return;
}

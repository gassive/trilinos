//@HEADER
// ***********************************************************************
// 
//     EpetraExt: Epetra Extended - Linear Algebra Services Package
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
//@HEADER
#include "EpetraExt_RowMatrixOut.h"
#include "EpetraExt_mmio.h"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Epetra_Vector.h"
#include "Epetra_IntVector.h"
#include "Epetra_SerialDenseVector.h"
#include "Epetra_IntSerialDenseVector.h"
#include "Epetra_Import.h"
#include "Epetra_CrsMatrix.h"

using namespace EpetraExt;
namespace EpetraExt {

int RowMatrixToMatlabFile( const char *filename, const Epetra_RowMatrix & A) {

  // Simple wrapper to make it clear what can be used to write to Matlab format
  return(RowMatrixToMatrixMarketFile(filename, A, 0, 0, false));
}

int RowMatrixToMatrixMarketFile( const char *filename, const Epetra_RowMatrix & A, 
				 const char * matrixName,
				 const char *matrixDescription, 
				 bool writeHeader) {
  int M = A.NumGlobalRows();
  int N = A.NumGlobalCols();
  int nz = A.NumGlobalNonzeros();

  FILE * handle = 0;

  if (A.RowMatrixRowMap().Comm().MyPID()==0) { // Only PE 0 does this section

    handle = fopen(filename,"w");
    if (!handle) {EPETRA_CHK_ERR(-1);}
    MM_typecode matcode;
    mm_initialize_typecode(&matcode);
    mm_set_matrix(&matcode);
    mm_set_coordinate(&matcode);
    mm_set_real(&matcode);

    if (writeHeader==true) { // Only write header if requested (true by default)
    
      if (mm_write_banner(handle, matcode)!=0) {EPETRA_CHK_ERR(-1);}
      
      if (matrixName!=0) fprintf(handle, "%% \n%% %s\n", matrixName);
      if (matrixDescription!=0) fprintf(handle, "%% %s\n%% \n", matrixDescription);
      
      if (mm_write_mtx_crd_size(handle, M, N, nz)!=0) {EPETRA_CHK_ERR(-1);}
    }
  }
    
  if (RowMatrixToHandle(handle, A)!=0) {EPETRA_CHK_ERR(-1);}// Everybody calls this routine

  if (A.RowMatrixRowMap().Comm().MyPID()==0) // Only PE 0 opened a file
    if (fclose(handle)!=0) {EPETRA_CHK_ERR(-1);}
  return(0);
}

int RowMatrixToHandle(FILE * handle, const Epetra_RowMatrix & A) {

  Epetra_Map map = A.RowMatrixRowMap();
  const Epetra_Comm & comm = map.Comm();
  int numProc = comm.NumProc();

  if (numProc==1 || !A.Map().DistributedGlobal())
    writeRowMatrix(handle, A);
  else {
    int numRows = map.NumMyElements();
    
    Epetra_Map allGidsMap(-1, numRows, 0,comm);
    
    Epetra_IntVector allGids(allGidsMap);
    for (int i=0; i<numRows; i++) allGids[i] = map.GID(i);
    
    // Now construct a RowMatrix on PE 0 by strip-mining the rows of the input matrix A.
    int numChunks = numProc;
    int stripSize = allGids.GlobalLength()/numChunks;
    int remainder = allGids.GlobalLength()%numChunks;
    int curStart = 0;
    int curStripSize = 0;
    Epetra_IntSerialDenseVector importGidList;
    if (comm.MyPID()==0) 
      importGidList.Size(stripSize+1); // Set size of vector to max needed
    for (int i=0; i<numChunks; i++) {
      if (comm.MyPID()==0) { // Only PE 0 does this part
	curStripSize = stripSize;
	if (i<remainder) curStripSize++; // handle leftovers
	for (int j=0; j<curStripSize; j++) importGidList[j] = j + curStart;
	curStart += curStripSize;
      }
      // The following import map will be non-trivial only on PE 0.
      if (comm.MyPID()>0) assert(curStripSize==0);
      Epetra_Map importGidMap(-1, curStripSize, importGidList.Values(), 0, comm);
      Epetra_Import gidImporter(importGidMap, allGidsMap);
      Epetra_IntVector importGids(importGidMap);
      if (importGids.Import(allGids, gidImporter, Insert)!=0) {EPETRA_CHK_ERR(-1); }

      // importGids now has a list of GIDs for the current strip of matrix rows.
      // Use these values to build another importer that will get rows of the matrix.

      // The following import map will be non-trivial only on PE 0.
      Epetra_Map importMap(-1, importGids.MyLength(), importGids.Values(), map.IndexBase(), comm);
      Epetra_Import importer(importMap, map);
      Epetra_CrsMatrix importA(Copy, importMap, 0);
      if (importA.Import(A, importer, Insert)!=0) {EPETRA_CHK_ERR(-1); }
      if (importA.FillComplete(A.OperatorDomainMap(), importMap)!=0) {EPETRA_CHK_ERR(-1);}

      // Finally we are ready to write this strip of the matrix to ostream
      if (writeRowMatrix(handle, importA)!=0) {EPETRA_CHK_ERR(-1);}
    }
  }
  return(0);
}
int writeRowMatrix(FILE * handle, const Epetra_RowMatrix & A) {

  int numRows = A.NumGlobalRows();
  Epetra_Map rowMap = A.RowMatrixRowMap();
  Epetra_Map colMap = A.RowMatrixColMap();
  const Epetra_Comm & comm = rowMap.Comm();
  int ioffset = 1 - rowMap.IndexBase(); // Matlab indices start at 1
  int joffset = 1 - colMap.IndexBase(); // Matlab indices start at 1
  if (comm.MyPID()!=0) {
    if (A.NumMyRows()!=0) {EPETRA_CHK_ERR(-1);}
    if (A.NumMyCols()!=0) {EPETRA_CHK_ERR(-1);}
  }
  else {
    if (numRows!=A.NumMyRows()) {EPETRA_CHK_ERR(-1);}
    Epetra_SerialDenseVector values(A.MaxNumEntries());
    Epetra_IntSerialDenseVector indices(A.MaxNumEntries());
    for (int i=0; i<numRows; i++) {
      int I = rowMap.GID(i) + ioffset;
      int numEntries;
      if (A.ExtractMyRowCopy(i, values.Length(), numEntries, 
			     values.Values(), indices.Values())!=0) {EPETRA_CHK_ERR(-1);}
      for (int j=0; j<numEntries; j++) {
	int J = colMap.GID(indices[j]) + joffset;
	double val = values[j];
	fprintf(handle, "%d %d %22.16e\n", I, J, val);
      }
    }
  }
  return(0);
}
} // namespace EpetraExt

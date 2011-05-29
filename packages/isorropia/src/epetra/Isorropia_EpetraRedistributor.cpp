//@HEADER
/*
************************************************************************

              Isorropia: Partitioning and Load Balancing Package
                Copyright (2006) Sandia Corporation

Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
license for use of this work by or on behalf of the U.S. Government.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA

************************************************************************
*/
//@HEADER

#include <Isorropia_EpetraRedistributor.hpp>
#include <Isorropia_Exception.hpp>
#include <Isorropia_Epetra.hpp>
#include <Isorropia_EpetraPartitioner.hpp>

#include <Teuchos_RCP.hpp>

#ifdef HAVE_EPETRA
#include <Epetra_Map.h>
#include <Epetra_Import.h>
#include <Epetra_Vector.h>
#include <Epetra_MultiVector.h>
#include <Epetra_CrsGraph.h>
#include <Epetra_CrsMatrix.h>
#include <Epetra_LinearProblem.h>
#include <Epetra_Comm.h>
#endif

namespace Isorropia {

namespace Epetra {

#ifdef HAVE_EPETRA

Redistributor::Redistributor(Teuchos::RCP<Isorropia::Epetra::Partitioner> partitioner)
  : partitioner_(partitioner),
  importer_(),
  target_map_(),
  created_importer_(false)
{
  if (!partitioner_->alreadyComputed()) {
    partitioner_->partition();
  }
}

Redistributor::Redistributor(Isorropia::Epetra::Partitioner *partitioner)
  : partitioner_(Teuchos::RCP<Isorropia::Epetra::Partitioner>(partitioner,false)),
  importer_(),
  target_map_(),
  created_importer_(false)
{
  if (!partitioner_->alreadyComputed()) {
    partitioner_->partition();
  }
}

Redistributor::~Redistributor()
{
}

void
Redistributor::redistribute(const Epetra_SrcDistObject& src,
				 Epetra_DistObject& target)
{
  if (!created_importer_) {
    create_importer(src.Map());
  }
  else {
    if (!src.Map().PointSameAs(importer_->SourceMap()) ||
	!target.Map().PointSameAs(importer_->TargetMap())) {
      created_importer_ = false;
      create_importer(src.Map());
    }
  }

  target.Import(src, *importer_, Insert);
}

Teuchos::RCP<Epetra_CrsGraph>
Redistributor::redistribute(const Epetra_CrsGraph& input_graph, bool callFillComplete)
{
  Epetra_CrsGraph *outputGraphPtr=0;
  redistribute(input_graph, outputGraphPtr, callFillComplete);

  return Teuchos::RCP<Epetra_CrsGraph>(outputGraphPtr);
}

void 
Redistributor::redistribute(const Epetra_CrsGraph& input_graph, Epetra_CrsGraph * &outputGraphPtr, bool callFillComplete)
{
  if (!created_importer_) {
    create_importer(input_graph.RowMap());
  }

  // First obtain the length of each of my new rows

  int myOldRows = input_graph.NumMyRows();
  int myNewRows = target_map_->NumMyElements();

  double *nnz = new double [myOldRows];
  for (int i=0; i < myOldRows; i++){
    nnz[i] = input_graph.NumMyIndices(i);
  }

  Epetra_Vector oldRowSizes(Copy, input_graph.RowMap(), nnz);

  if (myOldRows)
    delete [] nnz;

  Epetra_Vector newRowSizes(*target_map_);

  newRowSizes.Import(oldRowSizes, *importer_, Insert);

  int *rowSize = new int [myNewRows];
  for (int i=0; i< myNewRows; i++){
    rowSize[i] = static_cast<int>(newRowSizes[i]);
  }

  // Receive new rows, send old rows

  outputGraphPtr = new Epetra_CrsGraph(Copy, *target_map_, rowSize, true);

  if (myNewRows)
    delete [] rowSize;

  outputGraphPtr->Import(input_graph, *importer_, Insert);

  // Set the new domain map such that
  // (a) if old DomainMap == old RangeMap, preserve this property,
  // (b) otherwise, let the new DomainMap be the old DomainMap 
  const Epetra_BlockMap *newDomainMap;
  if (input_graph.DomainMap().SameAs(input_graph.RangeMap()))
     newDomainMap = &(outputGraphPtr->RangeMap());
  else
     newDomainMap = &(input_graph.DomainMap());

  if (callFillComplete && (!outputGraphPtr->Filled()))
    outputGraphPtr->FillComplete(*newDomainMap, *target_map_);

  return;
}

Teuchos::RCP<Epetra_CrsMatrix>
Redistributor::redistribute(const Epetra_CrsMatrix& inputMatrix, bool callFillComplete)
{
  Epetra_CrsMatrix *outputMatrix=0;
  redistribute(inputMatrix, outputMatrix, callFillComplete);

  return Teuchos::RCP<Epetra_CrsMatrix>(outputMatrix);
}

void 
Redistributor::redistribute(const Epetra_CrsMatrix& inputMatrix, Epetra_CrsMatrix * &outputMatrix, bool callFillComplete)
{
  if (!created_importer_) {
    create_importer(inputMatrix.RowMap());
  }

  // First obtain the length of each of my new rows

  int myOldRows = inputMatrix.NumMyRows();
  int myNewRows = target_map_->NumMyElements();

  double *nnz = new double [myOldRows];
  for (int i=0; i < myOldRows; i++){
    nnz[i] = inputMatrix.NumMyEntries(i);
  }

  Epetra_Vector oldRowSizes(Copy, inputMatrix.RowMap(), nnz);

  if (myOldRows)
    delete [] nnz;

  Epetra_Vector newRowSizes(*target_map_);

  newRowSizes.Import(oldRowSizes, *importer_, Insert);

  int *rowSize=0;
  if(myNewRows){
    rowSize = new int [myNewRows];
    for (int i=0; i< myNewRows; i++){
      rowSize[i] = static_cast<int>(newRowSizes[i]);
    }
  }

  // Receive new rows, send old rows

  outputMatrix = new Epetra_CrsMatrix(Copy, *target_map_, rowSize, true);

  if (myNewRows)
    delete [] rowSize;

  outputMatrix->Import(inputMatrix, *importer_, Insert);

  // Set the new domain map such that
  // (a) if old DomainMap == old RangeMap, preserve this property,
  // (b) otherwise, let the new DomainMap be the old DomainMap 
  const Epetra_Map *newDomainMap;
  if (inputMatrix.DomainMap().SameAs(inputMatrix.RangeMap()))
     newDomainMap = &(outputMatrix->RangeMap());
  else
     newDomainMap = &(inputMatrix.DomainMap());

  if (callFillComplete && (!outputMatrix->Filled()))
    outputMatrix->FillComplete(*newDomainMap,  *target_map_);

  return;
}

Teuchos::RCP<Epetra_CrsMatrix>
Redistributor::redistribute(const Epetra_RowMatrix& inputMatrix, bool callFillComplete)
{
  Epetra_CrsMatrix *outputMatrix = 0;
  redistribute(inputMatrix,outputMatrix,callFillComplete);
  return Teuchos::RCP<Epetra_CrsMatrix>(outputMatrix);
}

void
Redistributor::redistribute(const Epetra_RowMatrix& inputMatrix, Epetra_CrsMatrix * &outputMatrix, bool callFillComplete)
{
  if (!created_importer_) {
    create_importer(inputMatrix.RowMatrixRowMap());
  }
 // First obtain the length of each of my new rows

  int myOldRows = inputMatrix.NumMyRows();
  int myNewRows = target_map_->NumMyElements();


  double *nnz = new double [myOldRows];
  int val;
  for (int i=0; i < myOldRows; i++){
    inputMatrix.NumMyRowEntries(i, val);
    nnz[i] = static_cast<double>(val);
  }

  Epetra_Vector oldRowSizes(Copy, inputMatrix.RowMatrixRowMap(), nnz);

  if (myOldRows)
    delete [] nnz;

  Epetra_Vector newRowSizes(*target_map_);

  newRowSizes.Import(oldRowSizes, *importer_, Insert);

  int *rowSize = new int [myNewRows];
  for (int i=0; i< myNewRows; i++){
    rowSize[i] = static_cast<int>(newRowSizes[i]);
  }

  // Receive new rows, send old rows

  outputMatrix = new Epetra_CrsMatrix(Copy, *target_map_, rowSize, true);

  if (myNewRows)
    delete [] rowSize;

  outputMatrix->Import(inputMatrix, *importer_, Insert);

  // Set the new domain map such that
  // (a) if old DomainMap == old RangeMap, preserve this property,
  // (b) otherwise, use the original OperatorDomainMap
  //if (inputMatrix.NumGlobalRows() == inputMatrix.NumGlobalCols()){
  if (inputMatrix.OperatorDomainMap().SameAs(inputMatrix.OperatorRangeMap())){
    if (callFillComplete && (!outputMatrix->Filled()))
      outputMatrix->FillComplete();
  }
  else {
    if (callFillComplete && (!outputMatrix->Filled()))
      outputMatrix->FillComplete(inputMatrix.OperatorDomainMap(), *target_map_);
  }

  return;
}

Teuchos::RCP<Epetra_Vector>
Redistributor::redistribute(const Epetra_Vector& input_vector)
{
  Epetra_Vector *outputVector = 0;
  redistribute(input_vector,outputVector);

  return Teuchos::RCP<Epetra_Vector>(outputVector);
}

void
Redistributor::redistribute(const Epetra_Vector& inputVector, Epetra_Vector * &outputVector)
{
  if (!created_importer_) {
    create_importer(inputVector.Map());
  }

  outputVector = new Epetra_Vector(*target_map_);

  outputVector->Import(inputVector, *importer_, Insert);

  return;
}

Teuchos::RCP<Epetra_MultiVector>
Redistributor::redistribute(const Epetra_MultiVector& input_vector)
{
  Epetra_MultiVector *outputVector=0;
  redistribute(input_vector,outputVector);

  return Teuchos::RCP<Epetra_MultiVector>(outputVector);
}


void
Redistributor::redistribute(const Epetra_MultiVector& inputVector, Epetra_MultiVector * &outputVector)
{
  if (!created_importer_) {
    create_importer(inputVector.Map());
  }

  outputVector = new Epetra_MultiVector(*target_map_, inputVector.NumVectors());

  outputVector->Import(inputVector, *importer_, Insert);

  return;
}











// Reverse redistribute methods (for vectors). 

void
Redistributor::redistribute_reverse(const Epetra_Vector& input_vector, Epetra_Vector& output_vector)
{
  if (!created_importer_) {
    create_importer(input_vector.Map());
  }

  // Export using the importer
  output_vector.Export(input_vector, *importer_, Insert);

}

void
Redistributor::redistribute_reverse(const Epetra_MultiVector& input_vector, Epetra_MultiVector& output_vector)
{
  if (!created_importer_) {
    create_importer(input_vector.Map());
  }

  // Export using the importer
  output_vector.Export(input_vector, *importer_, Insert);

}

void Redistributor::create_importer(const Epetra_BlockMap& src_map)
{
  if (created_importer_) return;

  if (partitioner_->numProperties() > src_map.Comm().NumProc()) {
    throw Isorropia::Exception("Cannot redistribute: Too many parts for too few processors.");
  }

  target_map_ = partitioner_->createNewMap();

  importer_ = Teuchos::rcp(new Epetra_Import(*target_map_, src_map));

  created_importer_ = true;
}

#endif //HAVE_EPETRA

}//namespace Epetra

}//namespace Isorropia


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

#ifndef _Isorropia_Zoltan_Repartition_hpp_
#define _Isorropia_Zoltan_Repartition_hpp_

#include <Isorropia_ConfigDefs.hpp>

#ifdef HAVE_ISORROPIA_ZOLTAN

#include <Teuchos_RefCountPtr.hpp>
#include <Teuchos_ParameterList.hpp>

#include <vector>
#include <map>

#ifdef HAVE_MPI
#include <mpi.h>
#endif

#ifdef HAVE_EPETRA
class Epetra_CrsGraph;
class Epetra_RowMatrix;
#endif

#include <QueryObject.hpp>

// class Isorropia::Epetra::CostDescriber;

namespace Isorropia{

namespace Epetra {

/** The ZoltanLib namespace within the Epetra namespace contains the
    classes and functions that use the Zoltan library to partition an
    Epetra object.
*/

namespace ZoltanLib{

#ifdef HAVE_EPETRA

/** Partition an Epetra_CrsGraph using Zoltan.

    myNewElements lists the global IDs of the rows I will have
    under the new partitioning.

    exports is a map from my row global IDs to process receiving that
    row under the new partititioning.

    imports is a map from each row globalID that will be sent to me
    under the new partitioning to the process that currently owns it.

    Isorropia::Epetra::ZoltanLib::repartition() is called by the
    Isorropia::Epetra::Partitioner constructor when Zoltan is available.
*/
int
repartition(Teuchos::RefCountPtr<const Epetra_CrsGraph> input_graph,
	    Teuchos::RefCountPtr<const Isorropia::Epetra::CostDescriber> costs,
	    Teuchos::ParameterList& paramlist,
            std::vector<int>& myNewElements,
            std::map<int,int>& exports,
            std::map<int,int>& imports);


/** Partition an Epetra_RowMatrix using Zoltan.

    myNewElements lists the global IDs of the rows I will have
    under the new partitioning.

    exports is a map from my row global IDs to process receiving that
    row under the new partititioning.

    imports is a map from each row globalID that will be sent to me
    under the new partitioning to the process that currently owns it.

    Isorropia::Epetra::ZoltanLib::repartition() is called by the
    Isorropia::Epetra::Partitioner constructor when Zoltan is available.
*/
int
repartition(Teuchos::RefCountPtr<const Epetra_RowMatrix> input_matrix,
	    Teuchos::RefCountPtr<const Isorropia::Epetra::CostDescriber> costs,
	    Teuchos::ParameterList& paramlist,
            std::vector<int>& myNewElements,
            std::map<int,int>& exports,
            std::map<int,int>& imports);

#ifdef HAVE_MPI
/** load_balance() is called by Isorropia::Epetra::ZoltanLib::repartition().

    It sets up the Zoltan query functions and parameters and calls Zoltan
    to perform the partitioning.

    myNewElements lists the global IDs of the rows I will have
    under the new partitioning.

    exports is a map from my row global IDs to process receiving that
    row under the new partititioning.

    imports is a map from each row globalID that will be sent to me
    under the new partitioning to the process that currently owns it.
*/
    
int
load_balance(MPI_Comm &comm,
	     Teuchos::ParameterList& paramlist,
	     QueryObject & queryObject,
	     std::vector<int>& myNewElements,
	     std::map<int,int>& exports,
	     std::map<int,int>& imports);
#endif

#endif //HAVE_EPETRA

}//namespace ZoltanLib

}//namespace Epetra

}//namespace Isorropia

//the following endif closes the '#ifdef HAVE_ISORROPIA_ZOLTAN' block.
#endif

#endif


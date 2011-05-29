//@HEADER
// ************************************************************************
//
//               Isorropia: Partitioning and Load Balancing Package
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
//
// ************************************************************************
//@HEADER

//
// Read in the file "simple.coords" and perform geometric partitioning
// with Zoltan's RCB method.
//
// --f={filename} will read a different coordinate file
// --v will print out the partitioning (small coordinate files only)
// --rib will use Recursive Inertial Bisection instead of RCB
//
// The input file should be a
// text file containing 1, 2 or 3-dimensional coordinates, one per line,
// with white space separating coordinate values.
//
// There are a few other coordinate files in the test/geometric directory.

#include <sys/types.h>
#include <unistd.h>

#include <Isorropia_ConfigDefs.hpp>
#include <Isorropia_Epetra.hpp>
#include <Isorropia_EpetraPartitioner.hpp>
#include <Isorropia_EpetraRedistributor.hpp>

#define PRINTLIMIT 5000

#ifdef HAVE_EPETRA
#include <Epetra_Import.h>
#ifdef HAVE_MPI
#include <Epetra_MpiComm.h>
#else
#include <Epetra_SerialComm.h>
#endif
#include <Epetra_MultiVector.h>

#include <Teuchos_CommandLineProcessor.hpp>
#include <Teuchos_RCP.hpp>

#include <string>

#include "ispatest_lbeval_utils.hpp"
#include "ispatest_epetra_utils.hpp"
#ifdef _MSC_VER
#include "winprocess.h"
#endif

int main(int argc, char** argv) {

  int fail = 0, dim=0;  
  int localProc = 0;
  int numProcs = 1;

#ifdef HAVE_MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &localProc);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcs);
  const Epetra_MpiComm Comm(MPI_COMM_WORLD);
#else
  const Epetra_SerialComm Comm;
#endif

  if (getenv("DEBUGME")){
    std::cerr << localProc << " gdb test_rcb.exe " << getpid() << std::endl;
    sleep(15);
  }

  // =============================================================
  // get command line options
  // =============================================================

  Teuchos::CommandLineProcessor clp(false,true);

  std::string *inputFile = new std::string("simple.coords");
  bool verbose = false;
  bool rib = false;

  clp.setOption( "f", inputFile, "Name of coordinate input file");

  clp.setOption( "v", "q", &verbose,
                "Display coordinates and weights before and after partitioning.");

  clp.setOption( "rib", "rcb", &rib, "Run RIB instead of RCB");

  Teuchos::CommandLineProcessor::EParseCommandLineReturn parse_return =
    clp.parse(argc,argv);

  if( parse_return == Teuchos::CommandLineProcessor::PARSE_HELP_PRINTED){
#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 0;
  }
  if( parse_return != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL ) {
#ifdef HAVE_MPI
    MPI_Finalize();
#endif
    return 1;
  }

  // =============================================================
  // Open file of coordinates and distribute them across processes
  // =============================================================
  
  Epetra_MultiVector *mv = ispatest::file2multivector(Comm, *inputFile);

  if (!mv || ((dim = mv->NumVectors()) < 1)){
    if (localProc == 0)
      std::cerr << "Invalid input file " << *inputFile << std::endl;
    exit(1);
  }

  int vsize = mv->GlobalLength();

  if (verbose){
    if (vsize < PRINTLIMIT){
      ispatest::printMultiVector(*mv, std::cout, "Coordinates",PRINTLIMIT);
    }
    else{
      if (localProc == 0){
        std::cerr << "--v requested, but input file is larger than " << PRINTLIMIT << " coordinates." << std::endl;
        std::cerr << "Partitioning will be performed but will not be displayed." << std::endl;
        verbose = false;
      }
    }
  }

  // =============================================================
  // Create weights for coordinates - there are three different
  // functions in src/utils/ispatest_epetra_utils.cpp that can
  // create weights for the coordinates.
  // =============================================================

  //Epetra_MultiVector *wgts = ispatest::makeWeights(mv->Map(), &ispatest::unitWeights);
  Epetra_MultiVector *wgts = ispatest::makeWeights(mv->Map(), &ispatest::alternateWeights);
  //Epetra_MultiVector *wgts = ispatest::makeWeights(mv->Map(), &ispatest::veeWeights);

  Epetra_Vector * &w1 = (*wgts)(0);

  if (!wgts || ((dim = wgts->NumVectors()) != 1)){
    if (localProc == 0)
      std::cout << "can't create weights" << std::endl;
    exit(1);
  }

  if (verbose){ 
    ispatest::printMultiVector(*wgts, std::cout, "Weights",PRINTLIMIT);
  }

  // =============================================================
  //  Create a parameter list for Zoltan
  // =============================================================

  Teuchos::ParameterList params;

  if (rib){

   // This parameter specifies that partitioning should be performed
   // by a simple linear partitioner in Isorropia, rather than calling
   // upon Zoltan.

   params.set("Partitioning Method", "RIB");
   // params.set("PARTITIONING_METHOD", "RIB"); // same as above
  }
  else{

   // params.set("Partitioning Method", "RCB"); // default 

    // To set low-level Zoltan parameters, create a sublist titled "ZOLTAN"
    // with Zoltan parameters in it.  See the Zoltan Users' Guide for
    // Zoltan parameters.  http://www.cs.sandia.gov/Zoltan

    //Teuchos::ParameterList &sublist = params.sublist("ZOLTAN");
    //sublist.set("DEBUG_LEVEL", "1"); // Zoltan will print out parameters
    //sublist.set("DEBUG_LEVEL", "5");   // proc 0 will trace Zoltan calls
    //sublist.set("DEBUG_MEMORY", "2");  // Zoltan will trace alloc & free
  }
  // =============================================================
  // Create a partitioner, by default this will perform the partitioning as well
  // =============================================================

  Teuchos::RCP<const Epetra_MultiVector> mv_rcp = Teuchos::rcp(mv);
  Teuchos::RCP<const Epetra_MultiVector> wgts_rcp = Teuchos::rcp(wgts);

  Teuchos::RCP<Isorropia::Epetra::Partitioner> part =
    Teuchos::rcp(new Isorropia::Epetra::Partitioner(mv_rcp, wgts_rcp, params));

  // Create a Redistributor based on the partitioning

  Isorropia::Epetra::Redistributor rd(part);

  // Redistribute the coordinates

  Teuchos::RCP<Epetra_MultiVector> new_mv = rd.redistribute(*mv);

  if (verbose){
    ispatest::printMultiVector(*new_mv, std::cout, "New Coordinates", PRINTLIMIT);
  }

  // Redistribute the weights

  Teuchos::RCP<Epetra_MultiVector> new_wgts = rd.redistribute(*wgts);

  Epetra_Vector * &new_w1 = (*new_wgts)(0);

  if (verbose){
    ispatest::printMultiVector(*new_wgts, std::cout, "New Weights", PRINTLIMIT);
  }

  // =============================================================
  // Compute balance both before and after partitioning
  // =============================================================

  double min1, min2, max1, max2, avg1, avg2;
  double goal = 1.0 / (double)numProcs;

  ispatest::compute_balance(*w1, goal, min1, max1, avg1);

  if (localProc == 0){
    std::cout << "Balance before partitioning: min " ;
    std::cout << min1 << " max " << max1 << " average " << avg1 << std::endl;
  }

  ispatest::compute_balance(*new_w1, goal, min2, max2, avg2);

  if (localProc == 0){
    std::cout << "Balance after partitioning:  min ";
    std::cout << min2 << " max " << max2 << " average " << avg2 << std::endl;
  }

#ifdef HAVE_MPI
  MPI_Finalize();
#endif

  return fail;
}

#endif   // HAVE_EPETRA

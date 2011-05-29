// @HEADER
// ***********************************************************************
//
//                 Anasazi: Block Eigensolvers Package
//                 Copyright (2010) Sandia Corporation
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

#include "AnasaziConfigDefs.hpp"

#include "Teuchos_ConfigDefs.hpp" // HAVE_MPI
#include "Teuchos_Tuple.hpp"
#ifdef HAVE_MPI
#  include "Teuchos_GlobalMPISession.hpp"
#  include "Teuchos_oblackholestream.hpp"
#endif // HAVE_MPI
#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_DefaultComm.hpp"
#include "Teuchos_Time.hpp"

#include "Tsqr_SeqTest.hpp"

#ifdef HAVE_TSQR_COMPLEX
#  include <complex>
#endif // HAVE_TSQR_COMPLEX

#include <sstream>
#include <stdexcept>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR { 
  namespace Trilinos { 
    namespace Test {

      const char docString[] = "This program tests TSQR::SequentialTsqr, "
	"which implements the sequential cache-blocked version of TSQR.  "
	"Accuracy and performance tests are included.";

      using Teuchos::RCP;
      using Teuchos::Tuple;

      /// \class SeqTestParameters
      /// \brief Encapsulates values of command-line parameters
      ///
      struct SeqTestParameters {
	SeqTestParameters () :
	  verify (true),
	  benchmark (false),
	  numRows (1000),
	  numCols (10),  
	  numTrials (10),
#ifdef HAVE_TSQR_COMPLEX
	  testComplex (true),
#endif // HAVE_TSQR_COMPLEX
	  cacheBlockSize (0),
	  contiguousCacheBlocks (false),
	  humanReadable (false),
	  debug (false)
	{}

	bool verify, benchmark;
	int numRows, numCols, numTrials;
#ifdef HAVE_TSQR_COMPLEX
	bool testComplex;
#endif // HAVE_TSQR_COMPLEX
	size_t cacheBlockSize;
	bool contiguousCacheBlocks, humanReadable, debug;
      };

      static void
      benchmarkSeqTsqr (std::ostream& out,
			const SeqTestParameters& params)
      {
	typedef Teuchos::Time timer_type;
#ifdef HAVE_TSQR_COMPLEX
	const bool testComplex = params.testComplex;
#else
	const bool testComplex = false;
#endif // HAVE_TSQR_COMPLEX
	TSQR::Test::benchmarkSeqTsqr< timer_type > (out, 
						    params.numRows, 
						    params.numCols, 
						    params.numTrials, 
						    params.cacheBlockSize,
						    params.contiguousCacheBlocks,
						    testComplex, 
						    params.humanReadable);
      }

      static void
      verifySeqTsqr (std::ostream& out,
		     const SeqTestParameters& params)
      {
#ifdef HAVE_TSQR_COMPLEX
	const bool testComplex = params.testComplex;
#else
	const bool testComplex = false;
#endif // HAVE_TSQR_COMPLEX
	const bool saveMatrices = false;
	TSQR::Test::verifySeqTsqr (out, params.numRows, params.numCols, 
				   params.cacheBlockSize, testComplex,
				   saveMatrices, params.contiguousCacheBlocks,
				   params.humanReadable, params.debug);
      }

      /// \brief Parse command-line options for this test
      ///
      /// \param argc [in] As usual in C(++)
      /// \param argv [in] As usual in C(++)
      /// \param allowedToPrint [in] Whether this (MPI) process is allowed
      ///   to print to stdout/stderr.  Different per (MPI) process.
      /// \param printedHelp [out] Whether this (MPI) process printed the
      ///   "help" display (summary of command-line options)
      ///
      /// \return Encapsulation of command-line options 
      static SeqTestParameters
      parseOptions (int argc, 
		    char* argv[], 
		    const bool allowedToPrint, 
		    bool& printedHelp)
      {
	using std::cerr;
	using std::endl;

	printedHelp = false;

	// Command-line parameters, set to their default values.
	SeqTestParameters params;
	/// We really want the cache block size as a size_t, but
	/// Teuchos::CommandLineProcessor doesn't offer that option.
	/// So we read it in as an int, which means negative inputs
	/// are possible.  We check for those below in the input
	/// validation phase.
	//
	// Fetch default value of cacheBlockSize.
	int cacheBlockSizeAsInt = static_cast<int> (params.cacheBlockSize);
	try {
	  using Teuchos::CommandLineProcessor;

	  CommandLineProcessor cmdLineProc (/* throwExceptions=*/ true, 
					    /* recognizeAllOptions=*/ true);
	  cmdLineProc.setDocString (docString);
	  cmdLineProc.setOption ("verify",
				 "noverify",
				 &params.verify,
				 "Test accuracy");
	  cmdLineProc.setOption ("benchmark",
				 "nobenchmark",
				 &params.benchmark,
				 "Test performance");
	  cmdLineProc.setOption ("nrows", 
				 &params.numRows, 
				 "Number of rows in the test matrix");
	  cmdLineProc.setOption ("ncols", 
				 &params.numCols, 
				 "Number of columns in the test matrix");
	  cmdLineProc.setOption ("ntrials", 
				 &params.numTrials, 
				 "Number of trials (only used when \"--benchmark\"");
#ifdef HAVE_TSQR_COMPLEX
	  cmdLineProc.setOption ("complex", 
				 "nocomplex",
				 &params.testComplex,
				 "Test complex arithmetic, as well as real");
#endif // HAVE_TSQR_COMPLEX
	  cmdLineProc.setOption ("cache-block-size", 
				 &cacheBlockSizeAsInt, 
				 "Cache block size in bytes (0 means pick a reasonable default)");
	  cmdLineProc.setOption ("contiguous-cache-blocks",
				 "noncontiguous-cache-blocks",
				 &params.contiguousCacheBlocks,
				 "Whether cache blocks should be stored contiguously");
	  cmdLineProc.setOption ("human-readable",
				 "machine-readable",
				 &params.humanReadable,
				 "If set, make output easy to read by humans "
				 "(but hard to parse)");
	  cmdLineProc.setOption ("debug", 
				 "nodebug", 
				 &params.debug, 
				 "Print debugging information");
	  cmdLineProc.parse (argc, argv);
	} 
	catch (Teuchos::CommandLineProcessor::UnrecognizedOption& e) { 
	  if (allowedToPrint)
	    cerr << "Unrecognized command-line option: " << e.what() << endl;
	  throw e;
	}
	catch (Teuchos::CommandLineProcessor::HelpPrinted& e) { 
	  printedHelp = true; 
	  return params; // Don't verify parameters in this case
	} 

	// Validate command-line options.  We provide default values
	// for unset options, so we don't have to validate those.
	if (params.numRows <= 0)
	  throw std::invalid_argument ("Number of rows must be positive");
	else if (params.numCols <= 0)
	  throw std::invalid_argument ("Number of columns must be positive");
	else if (params.numRows < params.numCols)
	  throw std::invalid_argument ("Number of rows must be >= number of columns");
	else if (params.benchmark && params.numTrials < 1)
	  throw std::invalid_argument ("\"--benchmark\" option requires numTrials >= 1");
	else
	  {
	    if (cacheBlockSizeAsInt < 0)
	      throw std::invalid_argument ("Cache block size must be nonnegative");
	    else 
	      params.cacheBlockSize = static_cast< size_t > (cacheBlockSizeAsInt);
	  }
	return params;
      }

    } // namespace Test
  } // namespace Trilinos
} // namespace TSQR


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int 
main (int argc, char *argv[]) 
{
  using Teuchos::RCP;
  using TSQR::Trilinos::Test::SeqTestParameters;
  using TSQR::Trilinos::Test::parseOptions;

#ifdef HAVE_MPI
  typedef RCP< const Teuchos::Comm<int> > comm_ptr;

  Teuchos::oblackholestream blackhole;
  Teuchos::GlobalMPISession mpiSession (&argc, &argv, &blackhole);
  comm_ptr comm = Teuchos::DefaultComm<int>::getComm();
  const int myRank = comm->getRank();
  // Only Rank 0 gets to write to stdout.  The other MPI process ranks
  // send their output to something that looks like /dev/null (and
  // likely is, on Unix-y operating systems).
  std::ostream& out = (myRank == 0) ? std::cout : blackhole;
  // Only Rank 0 performs the tests.
  const bool performingTests = (myRank == 0);
  const bool allowedToPrint = (myRank == 0);

#else // Don't HAVE_MPI: single-node test

  const bool performingTests = true;
  const bool allowedToPrint = true;
  std::ostream& out = std::cout;
#endif // HAVE_MPI

  // Fetch command-line parameters.
  bool printedHelp = false;
  SeqTestParameters params = 
    parseOptions (argc, argv, allowedToPrint, printedHelp);
  if (printedHelp)
    return 0;

  if (performingTests)
    {
      using std::endl;

      if (params.benchmark)
	TSQR::Trilinos::Test::benchmarkSeqTsqr (out, params);
      // We allow the same run to do both benchmark and verify.
      if (params.verify)
	TSQR::Trilinos::Test::verifySeqTsqr (out, params);

      // The Trilinos test framework expects a message like this.
      out << "\nEnd Result: TEST PASSED" << endl;
    }

  return 0;
}



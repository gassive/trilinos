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

#ifndef __TSQR_Test_CombineBenchmark_hpp
#define __TSQR_Test_CombineBenchmark_hpp

#include <Tsqr_Config.hpp>
#include <Tsqr_CombineBenchmarker.hpp>

#include <Tsqr_CombineDefault.hpp>
#include <Tsqr_CombineNative.hpp>
#ifdef HAVE_TSQR_FORTRAN
#  include <Tsqr_CombineFortran.hpp>
#endif // HAVE_TSQR_FORTRAN

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {
  namespace Test {

    template< class CombineType, class TimerType >
    static void
    benchmarkCombineType (std::ostream& out,
			  std::vector<int>& iseed,
			  const std::string& dataTypeName,
			  const std::string& combineTypeName,
			  const typename CombineType::ordinal_type numRows,
			  const typename CombineType::ordinal_type numCols,
			  const int numTrials)
    {
      using TSQR::Random::NormalGenerator;
      using std::complex;
      using std::endl;
      using std::pair;
      using std::string;
      using std::vector;

      typedef typename CombineType::ordinal_type ordinal_type;
      typedef typename CombineType::scalar_type scalar_type;
      typedef typename CombineType::magnitude_type magnitude_type;
      typedef CombineBenchmarker< ordinal_type, scalar_type, CombineType, TimerType > benchmarker_type;
      typedef pair< double, double > results_type;

      // FIXME These two should have different seeds!!!
      NormalGenerator< ordinal_type, scalar_type > normGen (iseed);
      NormalGenerator< ordinal_type, magnitude_type > normMagGen (iseed);

      benchmarker_type benchmarker (normGen, normMagGen, numTrials);
      results_type results (benchmarker.R1R2_benchmark (numCols),
			    benchmarker.RA_benchmark (numRows, numCols));
      out << combineTypeName 
	  << "," << dataTypeName
	  << "," << "R1_R2"
	  << "," << numRows
	  << "," << numCols
	  << "," << numTrials
	  << "," << results.first
	  << endl;
      out << combineTypeName 
	  << "," << dataTypeName
	  << "," << "R_A"
	  << "," << numRows
	  << "," << numCols
	  << "," << numTrials
	  << "," << results.second
	  << endl;
    }

    template< class Scalar, class TimerType >
    static void
    benchmarkAllCombineTypes (std::ostream& out,
			      std::vector<int>& iseed,
			      const std::string& dataTypeName,
			      const int numRows,
			      const int numCols,
			      const int numTrials)
    {
      using std::string;

      {
	typedef CombineNative< int, Scalar > combine_type;
	string combineTypeName ("Native");
	benchmarkCombineType< combine_type, TimerType > (out, iseed, dataTypeName, combineTypeName, numRows, numCols, numTrials);
      }
#ifdef HAVE_TSQR_FORTRAN
      {
	typedef CombineFortran< Scalar > combine_type;
	string combineTypeName ("Fortran");
	benchmarkCombineType< combine_type, TimerType > (out, iseed, dataTypeName, combineTypeName, numRows, numCols, numTrials);
      }
#endif // HAVE_TSQR_FORTRAN
      {
	typedef CombineDefault< int, Scalar > combine_type;
	string combineTypeName ("Default");
	benchmarkCombineType< combine_type, TimerType > (out, iseed, dataTypeName, combineTypeName, numRows, numCols, numTrials);
      }
    }


    template< class TimerType >
    static void
    benchmarkAllCombineTypesAndScalars (std::ostream& out,
					std::vector<int>& iseed,
					const int numRows,
					const int numCols,
					const int numTrials,
					const bool testComplex)
    {
      using std::complex;
      using std::string;
      string dataTypeName;

      dataTypeName = "float";
      benchmarkAllCombineTypes< float, TimerType > (out, iseed, dataTypeName, numRows, numCols, numTrials);
      dataTypeName = "double";
      benchmarkAllCombineTypes< double, TimerType > (out, iseed, dataTypeName, numRows, numCols, numTrials);

      if (testComplex)
	{
#ifdef HAVE_TSQR_COMPLEX
	  dataTypeName = "complex<float>";
	  benchmarkAllCombineTypes< complex<float>, TimerType > (out, iseed, dataTypeName, 
								 numRows, numCols, numTrials);
	  dataTypeName = "complex<double>";
	  benchmarkAllCombineTypes< complex<double>, TimerType > (out, iseed, dataTypeName, 
								  numRows, numCols, numTrials);
#else // Don't HAVE_TSQR_COMPLEX
	  throw std::logic_error("TSQR not built with complex arithmetic support");
#endif // HAVE_TSQR_COMPLEX
	}
    }					


    template< class TimerType >
    void
    benchmarkCombine (std::ostream& out,
		      const int numRows,
		      const int numCols,
		      const int numTrials,
		      std::vector<int>& seed,
		      const bool useSeedValues,
		      const bool testComplex)
    {
      if (! useSeedValues)
	{
	  seed.resize (4);
	  seed[0] = 0;
	  seed[1] = 0;
	  seed[2] = 0;
	  seed[3] = 1;
	}
      benchmarkAllCombineTypesAndScalars< TimerType > (out, seed, numRows, numCols, numTrials, testComplex);
    }

  } // namespace Test
} // namespace TSQR

#endif // __TSQR_Test_CombineBenchmark_hpp

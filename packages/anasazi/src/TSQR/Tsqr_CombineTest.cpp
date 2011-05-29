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

#include "Tsqr_CombineTest.hpp"

#include <Tsqr_Random_NormalGenerator.hpp>
#include <Tsqr_Random_MatrixGenerator.hpp>

#include <Tsqr_Combine.hpp>
#include <Tsqr_LocalVerify.hpp>
#include <Tsqr_Matrix.hpp>
#include <Tsqr_ScalarTraits.hpp>
#include <Tsqr_Util.hpp>

#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {
  namespace Test {

    template< class Ordinal, class MagnitudeType, class NormalGenType >
    static void 
    generateSingularValues (NormalGenType& magGen,
			    std::vector< MagnitudeType >& sigma,
			    const Ordinal numValues)
    {
      typedef MagnitudeType magnitude_type;
      const magnitude_type machEps = 
	std::numeric_limits< magnitude_type >::epsilon();
      sigma.resize (numValues);
    
      // Relative amount by which to perturb each singular value.  The
      // perturbation will be multiplied by a normal(0,1) pseudorandom
      // number drawn from magGen.
      const magnitude_type perturbationFactor = magnitude_type(10) * machEps;

      sigma[0] = magnitude_type (1);
      for (Ordinal k = 1; k < numValues; ++k)
	{
	  const magnitude_type perturbation = perturbationFactor * magGen();
	  const magnitude_type beforePerturb = sigma[k-1] / magnitude_type(2);
	  const magnitude_type candidate = beforePerturb + perturbation;

	  // If adding the perturbation to beforePerturb would result
	  // in a nonpositive number, subtract instead.
	  if (candidate <= magnitude_type(0))
	    sigma[k] = beforePerturb - perturbation;
	  else
	    sigma[k] = candidate;
	}
    }

    template< class MagnitudeType >
    static void
    printR1R2results (const std::string& datatype,
		      const int numCols,
		      const std::vector< MagnitudeType >& results)
    {
      using std::cout;
      using std::endl;

      cout << "Combine"
	   << "," << datatype
	   << "," << "R1R2"
	   << "," << numCols
	   << "," << numCols
	   << "," << results[0]
	   << "," << results[1]
	   << endl;
    }

    template< class MagnitudeType >
    static void
    printR3Aresults (const std::string& datatype,
		     const int numRows,
		     const int numCols,
		     const std::vector< MagnitudeType >& results)
    {
      using std::cout;
      using std::endl;

      cout << "Combine"
	   << "," << datatype
	   << "," << "R3A"
	   << "," << numRows
	   << "," << numCols
	   << "," << results[2]
	   << "," << results[3]
	   << endl;
    }

    template< class MagnitudeType >
    static void
    printResults (const std::string& datatype,
		  const int numRows,
		  const int numCols,
		  const std::vector< MagnitudeType >& results)
    {
      printR1R2results (datatype, numCols, results);
      printR3Aresults (datatype, numRows, numCols, results);
    }

    template< class MagnitudeType >
    static void
    printSimSeqTsqrResults (const std::string& datatype, 
			    const int numRows, 
			    const int numCols, 
			    const std::pair< MagnitudeType, MagnitudeType >& results)
    {
      using std::cout;
      using std::endl;

      cout << "CombineSimSeqTsqr"
	   << "," << datatype
	   << "," << numRows
	   << "," << numCols
	   << "," << results.first
	   << "," << results.second
	   << endl;
    }

    template< class MatrixViewType >
    static void
    printMatrix (std::ostream& out,
		 const MatrixViewType& A)
    {
      print_local_matrix (out, A.nrows(), A.ncols(), A.get(), A.lda());
    }

    template< class MatrixViewType >
    static
    std::pair< typename ScalarTraits< typename MatrixViewType::scalar_type >::magnitude_type, 
	       typename ScalarTraits< typename MatrixViewType::scalar_type >::magnitude_type >
    localVerify (const MatrixViewType& A, 
		 const MatrixViewType& Q, 
		 const MatrixViewType& R)
    {
      return local_verify (A.nrows(), A.ncols(), A.get(), A.lda(), 
			   Q.get(), Q.lda(), R.get(), R.lda());
    }
    
    /// \brief Test accuracy of TSQR::Combine
    ///
    /// 1. [R1; R2] where R1 and R2 are both ncols by ncols upper
    ///    triangular.
    /// 
    /// 2. [R; A] where R is ncols by ncols upper triangular, and A is
    ///    nrows by ncols general dense.
    ///
    ///
    /// \return (relative residual, relative orthogonality) for each
    ///   test problem.
    ///
    template< class Ordinal, class Scalar >
    static std::vector< typename ScalarTraits< Scalar >::magnitude_type >
    verifyCombineTemplate (TSQR::Random::NormalGenerator< Ordinal, Scalar >& gen,
			   TSQR::Random::NormalGenerator< Ordinal, typename ScalarTraits< Scalar >::magnitude_type >& magGen,
			   const Ordinal numRows, 
			   const Ordinal numCols,
			   const bool debug)
    {
      using TSQR::Random::MatrixGenerator;
      using TSQR::Random::NormalGenerator;
      using std::cerr;
      using std::endl;
      using std::invalid_argument;
      using std::ostringstream;
      using std::pair;
      using std::vector;

      typedef typename ScalarTraits< Scalar >::magnitude_type magnitude_type;
      typedef NormalGenerator< Ordinal, Scalar > normgen_type;
      typedef NormalGenerator< Ordinal, magnitude_type > normgen_mag_type;
      typedef MatrixGenerator< Ordinal, Scalar, normgen_type > matgen_type;
      typedef Matrix< Ordinal, Scalar > matrix_type;
      typedef pair< magnitude_type, magnitude_type > results_type;

      if (numRows < numCols)
	{
	  ostringstream os;
	  os << "# rows < # columns is not allowed.  You specified # rows = " 
	     << numRows << " and # columns = " << numCols << ".";
	  throw invalid_argument (os.str());
	}
      else if (numCols == 0)
	throw invalid_argument ("ncols == 0 is not allowed");

      //
      // Generate four different sets of singular values.  Randomly
      // perturb them, but make sure all are positive.
      //
      vector< magnitude_type > sigma_R1 (numCols);
      vector< magnitude_type > sigma_R2 (numCols);
      vector< magnitude_type > sigma_R3 (numCols);
      vector< magnitude_type > sigma_A (numCols);
      generateSingularValues (magGen, sigma_R1, numCols);
      generateSingularValues (magGen, sigma_R2, numCols);
      generateSingularValues (magGen, sigma_R3, numCols);
      generateSingularValues (magGen, sigma_A, numCols);

      matrix_type R1 (numCols, numCols, Scalar(0));
      matrix_type R2 (numCols, numCols, Scalar(0));
      matrix_type R3 (numCols, numCols, Scalar(0));
      matrix_type A (numRows, numCols, Scalar(0));
      matgen_type matgen (gen);
      matgen.fill_random_R (numCols, R1.get(), R1.lda(), &sigma_R1[0]);
      matgen.fill_random_R (numCols, R2.get(), R2.lda(), &sigma_R2[0]);
      matgen.fill_random_R (numCols, R3.get(), R3.lda(), &sigma_R3[0]);
      matgen.fill_random_svd (numRows, numCols, A.get(), A.lda(), &sigma_A[0]);

      if (false && debug)
	{
	  cerr << endl << "First test problem:" << endl;
	  print_local_matrix (cerr, numCols, numCols, R1.get(), R1.lda());
	  print_local_matrix (cerr, numCols, numCols, R2.get(), R2.lda());
	  cerr << endl;

	  cerr << endl << "Second test problem:" << endl;
	  print_local_matrix (cerr, numCols, numCols, R3.get(), R3.lda());
	  print_local_matrix (cerr, numRows, numCols, A.get(), A.lda());
	  cerr << endl;
	}

      // Space to put the original test problem, expressed as one
      // dense matrix rather than in two blocks.  These will be deep
      // copies of the test problems, since the test problem matrices
      // will be overwritten by the factorizations.
      matrix_type A_R1R2 (Ordinal(2) * numCols, numCols, Scalar(0));
      matrix_type A_R3A (numRows + numCols, numCols, Scalar(0));

      // Copy [R1; R2] into A_R1R2.
      copy_matrix (numCols, numCols, &A_R1R2(0, 0), A_R1R2.lda(), 
		   R1.get(), R1.lda());
      copy_matrix (numCols, numCols, &A_R1R2(numCols, 0), A_R1R2.lda(), 
		   R2.get(), R2.lda());

      // Copy [R3; A] into A_R3A.
      copy_matrix (numCols, numCols, &A_R3A(0, 0), A_R3A.lda(), 
		   R3.get(), R3.lda());
      copy_matrix (numRows, numCols, &A_R3A(numCols, 0), A_R3A.lda(), 
		   A.get(), A.lda());

      // Space to put the explicit Q factors.
      matrix_type Q_R1R2 (Ordinal(2) * numCols, numCols, Scalar(0));
      matrix_type Q_R3A (numRows + numCols, numCols, Scalar(0));

      // Fill the explicit Q factor matrices with the first numCols
      // columns of the identity matrix.
      for (Ordinal k = 0; k < numCols; ++k)
	{
	  Q_R1R2(k, k) = Scalar(1);
	  Q_R3A(k, k) = Scalar(1);
	}

      // tau factor arrays, one for each factorization test.
      vector< Scalar > tau_R1R2 (numCols);
      vector< Scalar > tau_R3A (numCols);

      // Workspace array for factorization and applying the Q factor.
      // We recycle this workspace for all tests.
      vector< Scalar > work (numCols);

      if (debug)
	cerr << endl << "----------------------------------------" << endl
	     << "TSQR::Combine first test problem:" << endl
	     << "qr( [R1; R2] ), with R1 and R2 " << numCols 
	     << " by " << numCols << endl << endl;

      Combine< Ordinal, Scalar > combiner;      
      combiner.factor_pair (numCols, R1.get(), R1.lda(), R2.get(), R2.lda(),
			    &tau_R1R2[0], &work[0]);
      combiner.apply_pair (ApplyType("N"), numCols, numCols, 
			   R2.get(), R2.lda(), &tau_R1R2[0], 
			   &Q_R1R2(0, 0), Q_R1R2.lda(),
			   &Q_R1R2(numCols, 0), Q_R1R2.lda(),
			   &work[0]);
      if (debug)
	{
	  cerr << "Results of first test problem:" << endl;
	  cerr << "-- Copy of test problem:" << endl;
	  print_local_matrix (cerr, A_R1R2.nrows(), A_R1R2.ncols(), 
			      A_R1R2.get(), A_R1R2.lda());
	  cerr << endl << "-- Q factor:" << endl;
	  print_local_matrix (cerr, Q_R1R2.nrows(), Q_R1R2.ncols(), 
			      Q_R1R2.get(), Q_R1R2.lda());
	  cerr << endl << "-- R factor:" << endl;
	  print_local_matrix (cerr, R1.nrows(), R1.ncols(), 
			      R1.get(), R1.lda());
	  cerr << endl;
	}
      const results_type firstResults = 
	local_verify (A_R1R2.nrows(), A_R1R2.ncols(), 
		      A_R1R2.get(), A_R1R2.lda(),
		      Q_R1R2.get(), Q_R1R2.lda(),
		      R1.get(), R1.lda());
      if (debug)
	cerr << "\\| A - Q*R \\|_F = " << firstResults.first << endl
	     << "\\| I - Q'*Q \\|_F = " << firstResults.second << endl;

      if (debug)
	cerr << endl << "----------------------------------------" << endl
	     << "TSQR::Combine second test problem:" << endl
	     << "qr( [R3; A] ), with R3 " << numCols << " by " << numCols 
	     << " and A " << numRows << " by " << numCols << endl << endl;

      combiner.factor_inner (numRows, numCols, R3.get(), R3.lda(),
			     A.get(), A.lda(), &tau_R3A[0], &work[0]);
      combiner.apply_inner (ApplyType("N"), numRows, numCols, numCols,
			    A.get(), A.lda(), &tau_R3A[0], 
			    &Q_R3A(0, 0), Q_R3A.lda(),
			    &Q_R3A(numCols, 0), Q_R3A.lda(), 
			    &work[0]);
      if (debug)
	{
	  cerr << "Results of second test problem:" << endl;
	  cerr << "-- Copy of test problem:" << endl;
	  print_local_matrix (cerr, A_R3A.nrows(), A_R3A.ncols(), 
			      A_R3A.get(), A_R3A.lda());
	  cerr << endl << "-- Q factor:" << endl;
	  print_local_matrix (cerr, Q_R3A.nrows(), Q_R3A.ncols(), 
			      Q_R3A.get(), Q_R3A.lda());
	  cerr << endl << "-- R factor:" << endl;
	  print_local_matrix (cerr, R3.nrows(), R3.ncols(), 
			      R3.get(), R3.lda());
	  cerr << endl;
	}
      const results_type secondResults = 
	local_verify (A_R3A.nrows(), A_R3A.ncols(), 
		      A_R3A.get(), A_R3A.lda(),
		      Q_R3A.get(), Q_R3A.lda(),
		      R3.get(), R3.lda());
      if (debug)
	cerr << "\\| A - Q*R \\|_F = " << secondResults.first << endl
	     << "\\| I - Q'*Q \\|_F = " << secondResults.second << endl;

      vector< magnitude_type > finalResults (4);
      finalResults[0] = firstResults.first;
      finalResults[1] = firstResults.second;
      finalResults[2] = secondResults.first;
      finalResults[3] = secondResults.second;
      return finalResults;
    }




    /// \brief Simulate one combine step of Sequential TSQR
    ///
    template< class Ordinal, class Scalar >
    static std::pair< typename ScalarTraits< Scalar >::magnitude_type, typename ScalarTraits< Scalar >::magnitude_type >
    verifyCombineSeqTemplate (TSQR::Random::NormalGenerator< Ordinal, Scalar >& gen,
			      TSQR::Random::NormalGenerator< Ordinal, typename ScalarTraits< Scalar >::magnitude_type >& magGen,
			      const Ordinal numRows, 
			      const Ordinal numCols,
			      const bool debug)
    {
      using TSQR::Random::MatrixGenerator;
      using TSQR::Random::NormalGenerator;
      using std::cerr;
      using std::endl;
      using std::invalid_argument;
      using std::ostringstream;
      using std::pair;
      using std::vector;

      typedef typename ScalarTraits< Scalar >::magnitude_type magnitude_type;
      typedef NormalGenerator< Ordinal, Scalar > normgen_type;
      typedef NormalGenerator< Ordinal, magnitude_type > normgen_mag_type;
      typedef MatrixGenerator< Ordinal, Scalar, normgen_type > matgen_type;
      typedef Matrix< Ordinal, Scalar > matrix_type;
      typedef MatView< Ordinal, Scalar > mat_view_type;
      typedef pair< magnitude_type, magnitude_type > results_type;

      if (numRows < numCols)
	{
	  ostringstream os;
	  os << "# rows < # columns is not allowed.  You specified # rows = " 
	     << numRows << " and # columns = " << numCols << ".";
	  throw invalid_argument (os.str());
	}
      else if (numCols == 0)
	throw invalid_argument ("ncols == 0 is not allowed");

      // Generate two different sets of singular values. 
      vector< magnitude_type > sigma_A1 (numCols);
      vector< magnitude_type > sigma_A2 (numCols);
      generateSingularValues (magGen, sigma_A1, numCols);
      generateSingularValues (magGen, sigma_A2, numCols);

      // Matrix consisting of two cache blocks.
      matrix_type A (Ordinal(2)*numRows, numCols, Scalar(0));
      // Views of the two cache blocks.
      mat_view_type A1 (numRows, numCols, &A(0,0), A.lda());
      mat_view_type A2 (numRows, numCols, &A(numRows,0), A.lda());

      // Fill the two cache blocks with random test problems.
      matgen_type matgen (gen);
      matgen.fill_random_svd (numRows, numCols, A1.get(), A1.lda(), &sigma_A1[0]);
      matgen.fill_random_svd (numRows, numCols, A2.get(), A2.lda(), &sigma_A2[0]);

      if (false && debug)
	{
	  cerr << endl << "Test problem:" << endl;
	  cerr << endl << "Original matrix:" << endl;
	  printMatrix (cerr, A);
	  cerr << endl << "First cache block:" << endl;
	  printMatrix (cerr, A1);
	  cerr << endl << "Second cache block:" << endl;
	  printMatrix (cerr, A2);
	  cerr << endl;
	}

      // Copy of the resulting test problem, stored as one dense
      // matrix rather than as two blocks.  We will use A_copy to
      // measure the residual error once we've completed the
      // factorization and computed the explicit Q factor.
      matrix_type A_copy (A);

      // Space to put the explicit Q factor.
      matrix_type Q (Ordinal(2) * numRows, numCols, Scalar(0));

      // Fill Q with the first numCols columns of the identity matrix.
      for (Ordinal k = 0; k < numCols; ++k)
	Q(k, k) = Scalar(1);

      // Two cache blocks (as views) of Q.
      mat_view_type Q1 (numRows, numCols, &Q(0,0), Q.lda());
      mat_view_type Q2 (numRows, numCols, &Q(numRows,0), Q.lda());

      // Two tau factor arrays, one for each cache block.
      vector< Scalar > tau1 (numCols);
      vector< Scalar > tau2 (numCols);

      // Workspace array for factorization and applying the Q factor.
      // We recycle this workspace for all tests.
      vector< Scalar > work (numCols);

      if (debug)
	cerr << endl << "----------------------------------------" << endl
	     << "TSQR::Combine SequentialTsqr simulation with 2 cache blocks:" 
	     << endl << "qr( [A1; A2] ), with A1 and A2 being each "
	     << numRows << " by " << numCols << endl << endl;

      Combine< Ordinal, Scalar > combiner;      
      // qr( A1 )
      combiner.factor_first (numRows, numCols, A1.get(), A1.lda(), 
			     &tau1[0], &work[0]);
      // View of numCols by numCols upper triangle of A1.
      mat_view_type R1 (numCols, numCols, A1.get(), A1.lda());
      // qr( [R1; A2] )
      combiner.factor_inner (numRows, numCols, R1.get(), R1.lda(),
			     A2.get(), A2.lda(), &tau2[0], &work[0]);
      // Extract (a deep copy of) the R factor.
      matrix_type R (R1);
      // Zero out everything below the diagonal of R.
      for (Ordinal j = 0; j < numCols; ++j)
	for (Ordinal i = j+1; i < numCols; ++i)
	  R(i,j) = Scalar(0);

      // Compute the explicit Q factor, by starting with A2 and
      // (working up the matrix A,) finishing with A1.
      combiner.apply_inner (ApplyType::NoTranspose, 
			    numRows, numCols, numCols,
			    A2.get(), A2.lda(), &tau2[0], 
			    Q1.get(), Q1.lda(),
			    Q2.get(), Q2.lda(), &work[0]);
      combiner.apply_first (ApplyType::NoTranspose,
			    numRows, numCols, numCols,
			    A1.get(), A.lda(), &tau1[0],
			    Q1.get(), Q1.lda(), &work[0]);
      if (debug)
	{
	  cerr << "Results of first test problem:" << endl;
	  cerr << "-- Test matrix A:" << endl;
	  printMatrix (cerr, A_copy);
	  cerr << endl << "-- Q factor:" << endl;
	  printMatrix (cerr, Q);
	  cerr << endl << "-- R factor:" << endl;
	  printMatrix (cerr, R);
	  cerr << endl;
	}
      const results_type results = localVerify (A_copy, Q, R);

      if (debug)
	cerr << "\\| A - Q*R \\|_F = " << results.first << endl
	     << "\\| I - Q'*Q \\|_F = " << results.second << endl;

      return results;
    }


    void
    verifyCombine (const int numRows,
		   const int numCols,
		   const bool test_complex,
		   const bool simulate_sequential_tsqr,
		   const bool debug)
    {
      using TSQR::Random::NormalGenerator;
      using std::cerr;
      using std::complex;
      using std::cout;
      using std::endl;
      using std::pair;
      using std::string;
      using std::vector;

      //
      // We do tests one after another, using the seed from the
      // previous test in the current test, so that the pseudorandom
      // streams used by the tests are independent.
      //

      // On output: Seed for the next pseudorandom number generator.
      vector< int > iseed(4);

      if (! simulate_sequential_tsqr)
	{
	  // First test.  The PRNG seeds itself with a default value.
	  // This will be the same each time, so if you want
	  // nondeterministic behavior, you should pick the seed values
	  // yourself.
	  NormalGenerator< int, float > normgenS;
	  const vector< float > resultsS = 
	    verifyCombineTemplate (normgenS, normgenS, numRows, numCols, debug);
	  printResults (string("float"), numRows, numCols, resultsS);
	  // Fetch the pseudorandom seed from the previous test.
	  normgenS.getSeed (iseed);

	  if (test_complex)
	    {
	      // Next test.
	      NormalGenerator< int, complex<float> > normgenC (iseed);
	      const vector< float > resultsC = 
		verifyCombineTemplate (normgenC, normgenS, numRows, numCols, debug);
	      printResults (string("complex<float>"), numRows, numCols, resultsC);
	      // Fetch the seed from this test
	      normgenC.getSeed (iseed);
	    }

	  // Next test.
	  NormalGenerator< int, double > normgenD (iseed);
	  const vector< double > resultsD = 
	    verifyCombineTemplate (normgenD, normgenD, numRows, numCols, debug);
	  printResults (string("double"), numRows, numCols, resultsD);
	  normgenD.getSeed (iseed);

	  if (test_complex)
	    {
	      // Next test.
	      NormalGenerator< int, complex<double> > normgenZ (iseed);
	      const vector< double > resultsZ = 
		verifyCombineTemplate (normgenZ, normgenD, numRows, numCols, debug);
	      printResults (string("complex<double>"), numRows, numCols, resultsZ);
	    }
	}
      else // simulate_sequential_tsqr
	{
	  // First test.
	  NormalGenerator< int, float > normgenS2;
	  const pair< float, float > resultsS2 = 
	    verifyCombineSeqTemplate (normgenS2, normgenS2, numRows, numCols, debug);
	  printSimSeqTsqrResults (string("float"), numRows, numCols, resultsS2);
	  normgenS2.getSeed (iseed);

	  if (test_complex)
	    {
	      // Next test.
	      NormalGenerator< int, complex<float> > normgenC2 (iseed);
	      const pair< float, float > resultsC2 = 
		verifyCombineSeqTemplate (normgenC2, normgenS2, numRows, numCols, debug);
	      printSimSeqTsqrResults (string("complex<float>"), numRows, numCols, resultsC2);
	      normgenC2.getSeed (iseed);
	    }

	  // Next test.
	  NormalGenerator< int, double > normgenD2 (iseed);
	  const pair< double, double > resultsD2 = 
	    verifyCombineSeqTemplate (normgenD2, normgenD2, numRows, numCols, debug);
	  printSimSeqTsqrResults (string("double"), numRows, numCols, resultsD2);
	  normgenD2.getSeed (iseed);

	  if (test_complex)
	    {
	      // Next test.
	      NormalGenerator< int, complex<double> > normgenZ2 (iseed);
	      const pair< double, double > resultsZ2 = 
		verifyCombineSeqTemplate (normgenZ2, normgenD2, numRows, numCols, debug);
	      printSimSeqTsqrResults (string("complex<double>"), numRows, numCols, resultsZ2);
	    }
	}
    }

  } // namespace Test
} // namespace TSQR


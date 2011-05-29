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

#ifndef __TSQR_Tsqr_hpp
#define __TSQR_Tsqr_hpp

#include <Tsqr_ApplyType.hpp>
#include <Tsqr_Matrix.hpp>
#include <Tsqr_MessengerBase.hpp>
#include <Tsqr_DistTsqr.hpp>
#include <Tsqr_SequentialTsqr.hpp>
#include <Tsqr_ScalarTraits.hpp>
#include <Tsqr_Util.hpp>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {

  /// \class Tsqr
  /// \brief Parallel Tall Skinny QR (TSQR) factorization
  /// 
  /// Parallel Tall Skinny QR (TSQR) factorization of a matrix
  /// distributed in block rows across one or more MPI processes.  The
  /// parallel critical path length for TSQR is independent of the
  /// number of columns in the matrix, unlike ScaLAPACK's comparable
  /// QR factorization (P_GEQR2), Modified Gram-Schmidt, or Classical
  /// Gram-Schmidt.
  ///
  /// LocalOrdinal: index type that can address all elements of a
  /// matrix (when treated as a 1-D array, so for A[i + LDA*j], the
  /// number i + LDA*j must fit in a LocalOrdinal).
  ///
  /// Scalar: the type of the matrix entries.
  ///
  /// NodeTsqrType: the intranode (single-node) part of Tsqr.
  /// Defaults to sequential cache-blocked TSQR.  Any class
  /// implementing the same compile-time interface is valid.  We
  /// provide NodeTsqr.hpp as an archetype of the "NodeTsqrType"
  /// concept, but it is not necessary that NodeTsqrType derive from
  /// that abstract base class.
  ///
  /// DistTsqrType: the internode (across nodes) part of Tsqr.  Any
  /// class implementing the same compile-time interface as the
  /// default template parameter class is valid.
  ///
  /// \note TSQR only needs to know about the local ordinal type (used
  ///   to index matrix entries on a single node), not about the
  ///   global ordinal type (used to index matrix entries globally,
  ///   i.e., over all nodes).
  ///
  template< class LocalOrdinal, 
	    class Scalar, 
	    class NodeTsqrType = SequentialTsqr< LocalOrdinal, Scalar >,
	    class DistTsqrType = DistTsqr< LocalOrdinal, Scalar > >
  class Tsqr {
  public:
    typedef MatView< LocalOrdinal, Scalar > matview_type;
    typedef ConstMatView< LocalOrdinal, Scalar > const_matview_type;
    typedef Matrix< LocalOrdinal, Scalar > matrix_type;

    typedef Scalar scalar_type;
    typedef LocalOrdinal ordinal_type;
    typedef typename ScalarTraits< Scalar >::magnitude_type magnitude_type;

    typedef NodeTsqrType node_tsqr_type;
    typedef DistTsqrType dist_tsqr_type;
    typedef typename Teuchos::RCP< node_tsqr_type > node_tsqr_ptr;
    typedef typename Teuchos::RCP< dist_tsqr_type > dist_tsqr_ptr;
    typedef typename dist_tsqr_type::rank_type rank_type;

    typedef typename node_tsqr_type::FactorOutput NodeOutput;
    typedef typename dist_tsqr_type::FactorOutput DistOutput;
    typedef std::pair< NodeOutput, DistOutput > FactorOutput;

    /// Constructor
    ///
    /// \param nodeTsqr [in/out] Previously initialized NodeTsqrType
    ///   object.  This takes care of the intranode part of TSQR.
    ///
    /// \param distTsqr [in/out] Previously initialized DistTsqrType
    ///   object.  This takes care of the internode part of TSQR.
    Tsqr (const node_tsqr_ptr& nodeTsqr, const dist_tsqr_ptr& distTsqr) :
      nodeTsqr_ (nodeTsqr), distTsqr_ (distTsqr)
    {}

    /// \brief Cache block size in bytes
    ///
    /// Cache block size (in bytes) used by the underlying intranode
    /// TSQR implementation.  
    ///
    /// \note This value may differ from the cache block size given to
    ///   the constructor of the NodeTsqrType object, since that
    ///   constructor input is merely a suggestion.
    size_t cache_block_size() const { return nodeTsqr_->cache_block_size(); }

    /// Whether or not all diagonal entries of the R factor computed
    /// by the QR factorization are guaranteed to be nonnegative.
    ///
    /// \note This property holds if all QR factorization steps (both
    ///   intranode and internode) produce an R factor with a
    ///   nonnegative diagonal.
    bool QR_produces_R_factor_with_nonnegative_diagonal () const {
      return nodeTsqr_->QR_produces_R_factor_with_nonnegative_diagonal() &&
	distTsqr_->QR_produces_R_factor_with_nonnegative_diagonal();
    }


    void
    factorExplicit (matview_type& A, 
		    matview_type& Q, 
		    matview_type& R, 
		    const bool contiguousCacheBlocks = false)
    {
      // Sanity checks for matrix dimensions
      if (A.nrows() < A.ncols())
	throw std::logic_error ("A must have at least as many rows as columns");
      else if (A.nrows() != Q.nrows())
	throw std::logic_error ("A and Q must have same number of rows");
      else if (R.nrows() < R.ncols())
	throw std::logic_error ("R must have at least as many rows as columns");
      else if (A.ncols() != R.ncols())
	throw std::logic_error ("A and R must have the same number of columns");

      // Checks for quick exit, based on matrix dimensions
      if (Q.ncols() == ordinal_type(0))
	return;

      R.fill (scalar_type (0));
      NodeOutput nodeResults = 
	nodeTsqr_->factor (A.nrows(), A.ncols(), A.get(), A.lda(), 
			   R.get(), R.lda(), contiguousCacheBlocks);
      nodeTsqr_->fill_with_zeros (Q.nrows(), Q.ncols(), Q.get(), Q.lda(), 
				  contiguousCacheBlocks);
      matview_type Q_top_block = 
	nodeTsqr_->top_block (Q, contiguousCacheBlocks);
      if (Q_top_block.nrows() < R.ncols())
	throw std::logic_error("Q has too few rows");
      matview_type Q_top (R.ncols(), Q.ncols(), 
			  Q_top_block.get(), Q_top_block.lda());
      distTsqr_->factorExplicit (R, Q_top);
      nodeTsqr_->apply (ApplyType::NoTranspose, 
			A.nrows(), A.ncols(), A.get(), A.lda(), nodeResults,
			Q.ncols(), Q.get(), Q.lda(), contiguousCacheBlocks);
    }


    /// \brief Compute QR factorization of the global dense matrix A
    ///
    /// Compute the QR factorization of the tall and skinny dense
    /// matrix A.  The matrix A is distributed in a row block layout
    /// over all the MPI processes.  A_local contains the matrix data
    /// for this process.
    ///
    /// \param nrows_local [in] Number of rows of this node's local
    ///   component (A_local) of the matrix.  May differ on different
    ///   nodes.  Precondition: nrows_local >= ncols.
    ///
    /// \param ncols [in] Number of columns in the matrix to factor.
    ///   Should be the same on all nodes.
    ///   Precondition: nrows_local >= ncols.
    ///
    /// \param A_local [in,out] On input, this node's local component of
    ///   the matrix, stored as a general dense matrix in column-major
    ///   order.  On output, overwritten with an implicit representation
    ///   of the Q factor.
    ///
    /// \param lda_local [in] Leading dimension of A_local.  
    ///   Precondition: lda_local >= nrows_local.
    ///
    /// \param R [out] The final R factor of the QR factorization of the
    ///   global matrix A.  An ncols by ncols upper triangular matrix with
    ///   leading dimension ldr.
    ///
    /// \param ldr [in] Leading dimension of the matrix R.
    ///
    /// \param contiguousCacheBlocks [in] Whether or not cache blocks
    ///   of A_local are stored contiguously.  The default value of
    ///   false means that A_local uses ordinary column-major
    ///   (Fortran-style) order.  Otherwise, the details of the format
    ///   depend on the specific NodeTsqrType.  Tsqr's cache_block()
    ///   and un_cache_block() methods may be used to convert between
    ///   cache-blocked and non-cache-blocked (column-major order)
    ///   formats.
    ///
    /// \return Part of the representation of the implicitly stored Q
    ///   factor.  It should be passed into apply() or explicit_Q() as
    ///   the "factorOutput" parameter.  The other part of the
    ///   implicitly stored Q factor is stored in A_local (the input
    ///   is overwritten).  Both parts go together.
    FactorOutput
    factor (const LocalOrdinal nrows_local,
	    const LocalOrdinal ncols, 
	    Scalar A_local[],
	    const LocalOrdinal lda_local,
	    Scalar R[],
	    const LocalOrdinal ldr,
	    const bool contiguousCacheBlocks = false)
    {
      MatView< LocalOrdinal, Scalar > R_view (ncols, ncols, R, ldr);
      R_view.fill (Scalar(0));
      NodeOutput nodeResults = 
	nodeTsqr_->factor (nrows_local, ncols, A_local, lda_local, 
			  R_view.get(), R_view.lda(), 
			  contiguousCacheBlocks);
      DistOutput distResults = distTsqr_->factor (R_view);
      return std::make_pair (nodeResults, distResults);
    }

    /// \brief Apply Q factor to the global dense matrix C
    ///
    /// Apply the Q factor (computed by factor() and represented
    /// implicitly) to the global dense matrix C, consisting of all
    /// nodes' C_local matrices stacked on top of each other.
    ///
    /// \param [in] If "N", compute Q*C.  If "T", compute Q^T * C.
    ///   If "H" or "C", compute Q^H * C.  (The last option may not 
    ///   be implemented in all cases.)
    ///
    /// \param nrows_local [in] Number of rows of this node's local
    ///   component (C_local) of the matrix C.  Should be the same on
    ///   this node as the nrows_local argument with which factor() was
    ///   called  Precondition: nrows_local >= ncols.
    ///
    /// \param ncols_Q [in] Number of columns in Q.  Should be the same
    ///   on all nodes.  Precondition: nrows_local >= ncols_Q.
    ///
    /// \param Q_local [in] Same as A_local output of factor()
    ///
    /// \param ldq_local [in] Same as lda_local of factor()
    ///
    /// \param factor_output [in] Return value of factor()
    ///
    /// \param ncols_C [in] Number of columns in C.  Should be the same
    ///   on all nodes.  Precondition: nrows_local >= ncols_C.
    ///
    /// \param C_local [in,out] On input, this node's local component of
    ///   the matrix C, stored as a general dense matrix in column-major
    ///   order.  On output, overwritten with this node's component of 
    ///   op(Q)*C, where op(Q) = Q, Q^T, or Q^H.
    ///
    /// \param ldc_local [in] Leading dimension of C_local.  
    ///   Precondition: ldc_local >= nrows_local.
    ///
    /// \param contiguousCacheBlocks [in] Whether or not the cache
    ///   blocks of Q and C are stored contiguously.
    ///
    void
    apply (const std::string& op,
	   const LocalOrdinal nrows_local,
	   const LocalOrdinal ncols_Q,
	   const Scalar Q_local[],
	   const LocalOrdinal ldq_local,
	   const FactorOutput& factor_output,
	   const LocalOrdinal ncols_C,
	   Scalar C_local[],
	   const LocalOrdinal ldc_local,
	   const bool contiguousCacheBlocks = false)
    {
      ApplyType applyType (op);

      // This determines the order in which we apply the intranode
      // part of the Q factor vs. the internode part of the Q factor.
      const bool transposed = applyType.transposed();

      // View of this node's local part of the matrix C.
      matview_type C_view (nrows_local, ncols_C, C_local, ldc_local);

      // Identify top ncols_C by ncols_C block of C_local.
      // top_block() will set C_top_view to have the correct leading
      // dimension, whether or not cache blocks are stored
      // contiguously.
      //
      // C_top_view is the topmost cache block of C_local.  It has at
      // least as many rows as columns, but it likely has more rows
      // than columns.
      matview_type C_view_top_block = 
	nodeTsqr_->top_block (C_view, contiguousCacheBlocks);

      // View of the topmost ncols_C by ncols_C block of C.
      matview_type C_top_view (ncols_C, ncols_C, C_view_top_block.get(), 
			       C_view_top_block.lda());

      if (! transposed) 
	{
	  // C_top (small compact storage) gets a deep copy of the top
	  // ncols_C by ncols_C block of C_local.
      	  matrix_type C_top (C_top_view);

	  // Compute in place on all processors' C_top blocks.
	  distTsqr_->apply (applyType, C_top.ncols(), ncols_Q, C_top.get(), 
			    C_top.lda(), factor_output.second);

	  // Copy the result from C_top back into the top ncols_C by
	  // ncols_C block of C_local.
	  C_top_view.copy (C_top);

	  // Apply the local Q factor (in Q_local and
	  // factor_output.first) to C_local.
	  nodeTsqr_->apply (applyType, nrows_local, ncols_Q, 
			    Q_local, ldq_local, factor_output.first, 
			    ncols_C, C_local, ldc_local,
			    contiguousCacheBlocks);
	}
      else
	{
	  // Apply the (transpose of the) local Q factor (in Q_local
	  // and factor_output.first) to C_local.
	  nodeTsqr_->apply (applyType, nrows_local, ncols_Q, 
			    Q_local, ldq_local, factor_output.first, 
			    ncols_C, C_local, ldc_local,
			    contiguousCacheBlocks);

	  // C_top (small compact storage) gets a deep copy of the top
	  // ncols_C by ncols_C block of C_local.
      	  matrix_type C_top (C_top_view);

	  // Compute in place on all processors' C_top blocks.
	  distTsqr_->apply (applyType, ncols_C, ncols_Q, C_top.get(), 
			    C_top.lda(), factor_output.second);

	  // Copy the result from C_top back into the top ncols_C by
	  // ncols_C block of C_local.
	  C_top_view.copy (C_top);
	}
    }

    /// \brief Compute the explicit Q factor from factor()
    ///
    /// Compute the explicit version of the Q factor computed by
    /// factor() and represented implicitly (via Q_local_in and
    /// factor_output).
    ///
    /// \param nrows_local [in] Number of rows of this node's local
    ///   component (Q_local_in) of the matrix Q_local_in.  Also, the
    ///   number of rows of this node's local component (Q_local_out) of
    ///   the output matrix.  Should be the same on this node as the
    ///   nrows_local argument with which factor() was called.
    ///   Precondition: nrows_local >= ncols_Q_in.
    ///
    /// \param ncols_Q_in [in] Number of columns in the original matrix
    ///   A, whose explicit Q factor we are computing.  Should be the
    ///   same on all nodes.  Precondition: nrows_local >= ncols_Q_in.
    ///
    /// \param Q_local_in [in] Same as A_local output of factor().
    ///
    /// \param ldq_local_in [in] Same as lda_local of factor()
    ///
    /// \param factorOutput [in] Return value of factor().
    ///
    /// \param ncols_Q_out [in] Number of columns of the explicit Q
    ///   factor to compute.  Should be the same on all nodes.
    ///
    /// \param Q_local_out [out] This node's component of the Q factor
    ///   (in explicit form).
    ///
    /// \param ldq_local_out [in] Leading dimension of Q_local_out.
    ///
    /// \param contiguousCacheBlocks [in] Whether or not cache blocks
    ///   in Q_local_in and Q_local_out are stored contiguously.
    void
    explicit_Q (const LocalOrdinal nrows_local,
		const LocalOrdinal ncols_Q_in,
		const Scalar Q_local_in[],
		const LocalOrdinal ldq_local_in,
		const FactorOutput& factorOutput,
		const LocalOrdinal ncols_Q_out,
		Scalar Q_local_out[],
		const LocalOrdinal ldq_local_out,
		const bool contiguousCacheBlocks = false)
    {
      nodeTsqr_->fill_with_zeros (nrows_local, ncols_Q_out, Q_local_out,
				 ldq_local_out, contiguousCacheBlocks);
      const rank_type myRank = distTsqr_->rank();
      if (myRank == 0)
	{
	  // View of this node's local part of the matrix Q_out.
	  matview_type Q_out_view (nrows_local, ncols_Q_out, 
				   Q_local_out, ldq_local_out);

	  // View of the topmost cache block of Q_out.  It is
	  // guaranteed to have at least as many rows as columns.
	  matview_type Q_out_top = 
	    nodeTsqr_->top_block (Q_out_view, contiguousCacheBlocks);

	  // Fill (topmost cache block of) Q_out with the first
	  // ncols_Q_out columns of the identity matrix.
	  for (ordinal_type j = 0; j < ncols_Q_out; ++j)
	    Q_out_top(j, j) = Scalar(1);
	}
      apply ("N", nrows_local, 
	     ncols_Q_in, Q_local_in, ldq_local_in, factorOutput,
	     ncols_Q_out, Q_local_out, ldq_local_out,
	     contiguousCacheBlocks);
    }

    /// \brief Compute Q*B
    ///
    /// Compute matrix-matrix product Q*B, where Q is nrows by ncols
    /// and B is ncols by ncols.  Respect cache blocks of Q.
    void
    Q_times_B (const LocalOrdinal nrows,
	       const LocalOrdinal ncols,
	       Scalar Q[],
	       const LocalOrdinal ldq,
	       const Scalar B[],
	       const LocalOrdinal ldb,
	       const bool contiguousCacheBlocks = false) const
    {
      // This requires no internode communication.  However, the work
      // is not redundant, since each MPI process has a different Q.
      nodeTsqr_->Q_times_B (nrows, ncols, Q, ldq, B, ldb, 
			   contiguousCacheBlocks);
      // We don't need a barrier after this method, unless users are
      // doing mean MPI_Get() things.
    }

    /// Compute SVD \f$R = U \Sigma V^*\f$, not in place.  Use the
    /// resulting singular values to compute the numerical rank of R,
    /// with respect to the relative tolerance tol.  If R is full
    /// rank, return without modifying R.  If R is not full rank,
    /// overwrite R with \f$\Sigma \cdot V^*\f$.
    ///
    /// \return Numerical rank of R: 0 <= rank <= ncols.
    LocalOrdinal
    reveal_R_rank (const LocalOrdinal ncols,
		   Scalar R[],
		   const LocalOrdinal ldr,
		   Scalar U[],
		   const LocalOrdinal ldu,
		   const magnitude_type tol) const 
    {
      // Forward the request to the intranode TSQR implementation.
      // This work is performed redundantly on all MPI processes.
      //
      // FIXME (mfh 26 Aug 2010) Could be a problem if your cluster is
      // heterogeneous, because then you might obtain different
      // answers (due to rounding error) on different processors.
      return nodeTsqr_->reveal_R_rank (ncols, R, ldr, U, ldu, tol);
    }

    /// \brief Rank-revealing decomposition
    ///
    /// Using the R factor from factor() and the explicit Q factor
    /// from explicit_Q(), compute the SVD of R (\f$R = U \Sigma
    /// V^*\f$).  R.  If R is full rank (with respect to the given
    /// relative tolerance tol), don't change Q or R.  Otherwise,
    /// compute \f$Q := Q \cdot U\f$ and \f$R := \Sigma V^*\f$ in
    /// place (the latter may be no longer upper triangular).
    ///
    /// \param R [in/out] On input: ncols by ncols upper triangular
    /// matrix with leading dimension ldr >= ncols.  On output: if
    /// input is full rank, R is unchanged on output.  Otherwise, if
    /// \f$R = U \Sigma V^*\f$ is the SVD of R, on output R is
    /// overwritten with $\Sigma \cdot V^*$.  This is also an ncols by
    /// ncols matrix, but may not necessarily be upper triangular.
    ///
    /// \return Rank \f$r\f$ of R: \f$ 0 \leq r \leq ncols\f$.
    ///
    LocalOrdinal
    reveal_rank (const LocalOrdinal nrows,
		 const LocalOrdinal ncols,
		 Scalar Q[],
		 const LocalOrdinal ldq,
		 Scalar R[],
		 const LocalOrdinal ldr,
		 const magnitude_type tol,
		 const bool contiguousCacheBlocks = false) const
    {
      // Take the easy exit if available.
      if (ncols == 0)
	return 0;

      //
      // FIXME (mfh 16 Jul 2010) We _should_ compute the SVD of R (as
      // the copy B) on Proc 0 only.  This would ensure that all
      // processors get the same SVD and rank (esp. in a heterogeneous
      // computing environment).  For now, we just do this computation
      // redundantly, and hope that all the returned rank values are
      // the same.
      //
      matrix_type U (ncols, ncols, Scalar(0));
      const ordinal_type rank = 
	reveal_R_rank (ncols, R, ldr, U.get(), U.lda(), tol);
      
      if (rank < ncols)
	{
	  // cerr << ">>> Rank of R: " << rank << " < ncols=" << ncols << endl;
	  // cerr << ">>> Resulting U:" << endl;
	  // print_local_matrix (cerr, ncols, ncols, R, ldr);
	  // cerr << endl;

	  // If R is not full rank: reveal_R_rank() already computed
	  // the SVD \f$R = U \Sigma V^*\f$ of (the input) R, and
	  // overwrote R with \f$\Sigma V^*\f$.  Now, we compute \f$Q
	  // := Q \cdot U\f$, respecting cache blocks of Q.
	  Q_times_B (nrows, ncols, Q, ldq, U.get(), U.lda(), 
		     contiguousCacheBlocks);
	}
      return rank;
    }

    /// \brief Cache-block A_in into A_out
    ///
    /// Cache-block the given A_in matrix, writing the results to
    /// A_out.
    void
    cache_block (const LocalOrdinal nrows_local,
		 const LocalOrdinal ncols,
		 Scalar A_local_out[],
		 const Scalar A_local_in[],
		 const LocalOrdinal lda_local_in) const
    {
      nodeTsqr_->cache_block (nrows_local, ncols, 
			      A_local_out, 
			      A_local_in, lda_local_in);
    }

    /// \brief Un-cache-block A_in into A_out
    ///
    /// "Un"-cache-block the given A_in matrix, writing the results to
    /// A_out.
    void
    un_cache_block (const LocalOrdinal nrows_local,
		    const LocalOrdinal ncols,
		    Scalar A_local_out[],
		    const LocalOrdinal lda_local_out,		    
		    const Scalar A_local_in[]) const
    {
      nodeTsqr_->un_cache_block (nrows_local, ncols, 
				 A_local_out, lda_local_out, 
				 A_local_in);
    }

  private:
    node_tsqr_ptr nodeTsqr_;
    dist_tsqr_ptr distTsqr_;
  }; // class Tsqr

} // namespace TSQR

#endif // __TSQR_Tsqr_hpp

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

#ifndef __TSQR_Tsqr_NodeTsqr_hpp
#define __TSQR_Tsqr_NodeTsqr_hpp

#include <Tsqr_ApplyType.hpp>
#include <Tsqr_Lapack.hpp>
#include <Tsqr_MatView.hpp>
#include <Tsqr_ScalarTraits.hpp>
#include <cstring> // size_t

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {

  template< class LocalOrdinal, class Scalar >
  class NodeTsqrFactorOutput {
  public:
    NodeTsqrFactorOutput () {}
    virtual ~NodeTsqrFactorOutput() = 0;
  };

  template< class LocalOrdinal, class Scalar >
  class NodeTsqr {
  public:
    typedef Scalar scalar_type;
    typedef typename ScalarTraits< Scalar >::magnitude_type magnitude_type;
    typedef LocalOrdinal ordinal_type;
    typedef NodeTsqrFactorOutput factor_output_type;

    NodeTsqr(const size_t cacheBlockSize = 0) {}
    virtual ~NodeTsqr() {}

    /// Whether or not the R factor from the QR factorization has a
    /// nonnegative diagonal.
    bool QR_produces_R_factor_with_nonnegative_diagonal () const {
      return LAPACK< LocalOrdinal, Scalar >::QR_produces_R_factor_with_nonnegative_diagonal;
    }

    virtual size_t
    cache_block_size() const = 0;

    virtual void
    cache_block (const LocalOrdinal nrows,
		 const LocalOrdinal ncols, 
		 Scalar A_out[],
		 const Scalar A_in[],
		 const LocalOrdinal lda_in) const = 0;

    virtual void
    un_cache_block (const LocalOrdinal nrows,
		    const LocalOrdinal ncols,
		    Scalar A_out[],
		    const LocalOrdinal lda_out,		    
		    const Scalar A_in[]) const = 0;

    virtual factor_output_type
    factor (const LocalOrdinal nrows,
	    const LocalOrdinal ncols, 
	    Scalar A[],
	    const LocalOrdinal lda,
	    Scalar R[],
	    const LocalOrdinal ldr,
	    const bool contiguous_cache_blocks) = 0;

    virtual void
    apply (const ApplyType& apply_type,
	   const LocalOrdinal nrows,
	   const LocalOrdinal ncols_Q,
	   const Scalar* const Q,
	   const LocalOrdinal ldq,
	   const FactorOutput& factor_output,
	   const LocalOrdinal ncols_C,
	   Scalar* const C,
	   const LocalOrdinal ldc,
	   const bool contiguous_cache_blocks) = 0;

    virtual void
    explicit_Q (const LocalOrdinal nrows,
		const LocalOrdinal ncols_Q,
		const Scalar Q[],
		const LocalOrdinal ldq,
		const factor_output_type& factor_output,
		const LocalOrdinal ncols_C,
		Scalar C[],
		const LocalOrdinal ldc,
		const bool contiguous_cache_blocks) = 0;

    /// \brief Compute Q*B
    ///
    /// Compute matrix-matrix product Q*B, where Q is nrows by ncols
    /// and B is ncols by ncols.  Respect cache blocks of Q.
    virtual void
    Q_times_B (const LocalOrdinal nrows,
	       const LocalOrdinal ncols,
	       Scalar Q[],
	       const LocalOrdinal ldq,
	       const Scalar B[],
	       const LocalOrdinal ldb,
	       const bool contiguous_cache_blocks) const = 0;

    /// Compute SVD \f$R = U \Sigma V^*\f$, not in place.  Use the
    /// resulting singular values to compute the numerical rank of R,
    /// with respect to the relative tolerance tol.  If R is full
    /// rank, return without modifying R.  If R is not full rank,
    /// overwrite R with \f$\Sigma \cdot V^*\f$.
    ///
    /// \return Numerical rank of R: 0 <= rank <= ncols.
    virtual LocalOrdinal
    reveal_R_rank (const LocalOrdinal ncols,
		   Scalar R[],
		   const LocalOrdinal ldr,
		   Scalar U[],
		   const LocalOrdinal ldu,
		   const magnitude_type tol) const = 0;

    /// \brief Rank-revealing decomposition
    ///
    /// Using the R factor from factor() and the explicit Q factor
    /// from explicit_Q(), compute the SVD of R (\f$R = U \Sigma
    /// V^*\f$).  R.  If R is full rank (with respect to the given
    /// relative tolerance tol), don't change Q or R.  Otherwise,
    /// compute \f$Q := Q \cdot U\f$ and \f$R := \Sigma V^*\f$ in
    /// place (the latter may be no longer upper triangular).
    ///
    /// \return Rank \f$r\f$ of R: \f$ 0 \leq r \leq ncols\f$.
    ///
    virtual LocalOrdinal
    reveal_rank (const LocalOrdinal nrows,
		 const LocalOrdinal ncols,
		 Scalar Q[],
		 const LocalOrdinal ldq,
		 Scalar R[],
		 const LocalOrdinal ldr,
		 const magnitude_type tol,
		 const bool contiguous_cache_blocks) = 0;

    /// \brief Fill the nrows by ncols matrix A with zeros.
    /// 
    /// Fill the matrix A with zeros, in a way that respects the cache
    /// blocking scheme.
    ///
    /// \param nrows [in] Number of rows in A
    /// \param ncols [in] Number of columns in A 
    /// \param A [out] nrows by ncols column-major-order dense matrix 
    ///   with leading dimension lda
    /// \param lda [in] Leading dimension of A: lda >= nrows
    /// \param contiguous_cache_blocks [in] Whether the cache blocks
    ///   in A are stored contiguously.
    virtual void
    fill_with_zeros (const LocalOrdinal nrows,
		     const LocalOrdinal ncols,
		     Scalar A[],
		     const LocalOrdinal lda, 
		     const bool contiguous_cache_blocks = false) = 0;

    /// \brief Return topmost cache block of C
    ///
    /// \param C [in] Matrix (view), supporting the usual nrows(),
    ///   ncols(), get(), lda() interface.
    /// \param contiguous_cache_blocks [in] Whether the cache blocks
    ///   in C are stored contiguously.
    ///
    /// Return a view of the topmost cache block (on this node) of the
    /// given matrix C.  This is not necessarily square, though it
    /// must have at least as many rows as columns.  For a square
    /// ncols by ncols block, as needed by Tsqr::apply(), do as 
    /// follows:
    /// \code 
    /// MatrixViewType top = this->top_block (C, contig);
    /// MatView< LocalOrdinal, Scalar > square (ncols, ncols, top.get(), top.lda());
    /// \endcode
    template< class MatrixViewType >
    virtual MatrixViewType
    top_block (const MatrixViewType& C, 
	       const bool contiguous_cache_blocks = false) const = 0;
  };
} // namespace TSQR


#endif __TSQR_Tsqr_NodeTsqr_hpp

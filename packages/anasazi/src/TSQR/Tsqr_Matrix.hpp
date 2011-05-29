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

#ifndef __TSQR_Tsqr_Matrix_hpp
#define __TSQR_Tsqr_Matrix_hpp

#include <Tsqr_Util.hpp>
#include <Tsqr_MatView.hpp>

#include <stdexcept>
#include <sstream>
#include <limits>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {

  /// \class Matrix
  /// \brief A column-oriented dense matrix
  ///
  /// A column-oriented dense matrix, with indices of type Ordinal and
  /// elements of type Scalar.
  ///
  /// \note This class resembles Teuchos::SerialDenseMatrix.  It
  ///   exists because I didn't want TSQR to depend on Teuchos.  This
  ///   is not because I don't like Teuchos, but because I had/have
  ///   potential TSQR users who might not want to depend on Teuchos.
  template< class Ordinal, class Scalar >
  class Matrix {
  private:
    static bool 
    fits_in_size_t (const Ordinal& ord) 
    {
      const Ordinal result = static_cast< Ordinal > (static_cast< size_t > (ord));
      return (ord == result);
    }

    /// Check whether num_rows*num_cols makes sense as an amount of
    /// storage (for the num_rows by num_cols dense matrix).  Not
    /// making sense includes negative values for either parameter (if
    /// they are signed types), or overflow when computing their
    /// product.  Throw an exception of the appropriate type for any
    /// of these cases.  Otherwise, return num_rows*num_cols as a
    /// size_t.
    ///
    /// \param num_rows [in] Number of rows in the matrix
    /// \param num_cols [in] Number of columns in the matrix
    /// \return num_rows*num_cols
    size_t
    verified_alloc_size (const Ordinal num_rows,
			 const Ordinal num_cols) const 
    {
      if (! std::numeric_limits< Ordinal >::is_integer)
	throw std::logic_error("Ordinal must be an integer type");

      // Quick exit also checks for zero num_cols (which prevents
      // division by zero in the tests below).
      if (num_rows == 0 || num_cols == 0)
	return size_t(0);

      // If Ordinal is signed, make sure that num_rows and num_cols
      // are nonnegative.
      if (std::numeric_limits< Ordinal >::is_signed)
	{
	  if (num_rows < 0)
	    {
	      std::ostringstream os;
	      os << "# rows (= " << num_rows << ") < 0";
	      throw std::logic_error (os.str());
	    }
	  else if (num_cols < 0)
	    {
	      std::ostringstream os;
	      os << "# columns (= " << num_cols << ") < 0";
	      throw std::logic_error (os.str());
	    }
	}

      // If Ordinal is bigger than a size_t, do special range
      // checking.  The compiler warns (comparison of signed and
      // unsigned) if Ordinal is a signed type and we try to do
      // "numeric_limits<size_t>::max() <
      // std::numeric_limits<Ordinal>::max()", so instead we cast each
      // of num_rows and num_cols to size_t and back to Ordinal again,
      // and see if we get the same result.  If not, then we
      // definitely can't return a size_t product of num_rows and
      // num_cols.
      if (! fits_in_size_t (num_rows))
	{
	  std::ostringstream os;
	  os << "# rows (= " << num_rows << ") > max size_t value (= " 
	     << std::numeric_limits<size_t>::max() << ")";
	  throw std::range_error (os.str());
	}
      else if (! fits_in_size_t (num_cols))
	{
	  std::ostringstream os;
	  os << "# columns (= " << num_cols << ") > max size_t value (= "
	     << std::numeric_limits<size_t>::max() << ")";
	  throw std::range_error (os.str());
	}

      // Both num_rows and num_cols fit in a size_t, and are
      // nonnegative.  Now check whether their product also fits in a
      // size_t.  
      //
      // Note: This may throw a SIGFPE (floating-point exception) if
      // num_cols is zero.  Be sure to check first (above).
      if (static_cast<size_t>(num_rows) > 
	  std::numeric_limits<size_t>::max() / static_cast<size_t>(num_cols))
	{
	  std::ostringstream os;
	  os << "num_rows (= " << num_rows << ") * num_cols (= "
	     << num_cols << ") > max size_t value (= " 
	     << std::numeric_limits<size_t>::max() << ")";
	  throw std::range_error (os.str());
	}
      return static_cast<size_t>(num_rows) * static_cast<size_t>(num_cols);
    }
    
  public:
    typedef Scalar scalar_type;
    typedef Ordinal ordinal_type;
    typedef Scalar* pointer_type;

    Matrix (const Ordinal num_rows, 
	    const Ordinal num_cols) :
      nrows_ (num_rows),
      ncols_ (num_cols),
      A_ (verified_alloc_size (num_rows, num_cols))
    {}

    Matrix (const Ordinal num_rows,
	    const Ordinal num_cols,
	    const Scalar& value) :
      nrows_ (num_rows),
      ncols_ (num_cols),
      A_ (verified_alloc_size (num_rows, num_cols), value)
    {}

    // We need an explicit copy constructor, because for some reason
    // the default copy constructor (with shallow copies of pointers,
    // eeek! double free()s!!!) overrides the generic "copy
    // constructors" below.
    Matrix (const Matrix& in) :
      nrows_ (in.nrows()),
      ncols_ (in.ncols()),
      A_ (verified_alloc_size (in.nrows(), in.ncols()))
    {
      if (A_.size() > 0)
	copy_matrix (nrows(), ncols(), get(), lda(), in.get(), in.lda());
    }

    Matrix () : nrows_(0), ncols_(0), A_(0) {}

    ~Matrix () {} 

    template< class MatrixViewType >
    Matrix (const MatrixViewType& in) :
      nrows_ (in.nrows()),
      ncols_ (in.ncols()),
      A_ (verified_alloc_size (in.nrows(), in.ncols()))
    {
      if (A_.size() != 0)
	copy_matrix (nrows(), ncols(), get(), lda(), in.get(), in.lda());
    }

    /// *this gets a deep copy of B.
    ///
    /// \note Assumes *this and B have the same dimensions (but not
    ///   necessarily the same strides).
    template< class MatrixViewType >
    void 
    copy (MatrixViewType& B)
    {
      const typename MatrixViewType::ordinal_type num_rows = B.nrows();
      const typename MatrixViewType::ordinal_type num_cols = B.ncols();
      if (num_rows != nrows() || num_cols != ncols())
	{
	  std::ostringstream os;
	  os << "Matrix::Copy: incompatible dimensions: attempt to assign " 
	     << num_rows << " x " << num_cols << " matrix to an " 
	     << (nrows()) << " x " << (ncols()) << "matrix";
	  throw std::logic_error (os.str());
	}
      copy_matrix (nrows(), ncols(), get(), lda(), B.get(), B.lda());
    }

    void 
    fill (const Scalar value)
    {
      fill_matrix (nrows(), ncols(), get(), lda(), value);
    }

    /// 2-D Fortran array - style access (column-major order, though
    /// indexing is zero-based (C style) instead of one-based (Fortran
    /// style))
    Scalar& operator() (const Ordinal i, const Ordinal j) {
      return A_[i + j*lda()];
    }

    const Scalar& operator() (const Ordinal i, const Ordinal j) const {
      return A_[i + j*lda()];
    }

    /// 1-D std::vector - style access
    Scalar& operator[] (const Ordinal i) {
      return A_[i];
    }

    template< class MatrixViewType >
    bool operator== (const MatrixViewType& B) const 
    {
      if (nrows() != B.nrows() || ncols() != B.ncols())
	return false;
    
      typedef typename MatrixViewType::ordinal_type second_ordinal_type;
      typedef typename MatrixViewType::scalar_type second_scalar_type;
      typedef typename MatrixViewType::pointer_type second_pointer_type;

      const ordinal_type A_nrows = nrows();
      const ordinal_type A_lda = lda();
      const ordinal_type A_ncols = ncols();
      const second_ordinal_type B_lda = B.lda();
      const scalar_type* A_j = get();
      const second_scalar_type* B_j = B.get();

      for (ordinal_type j = 0; j < A_ncols; ++j, A_j += A_lda, B_j += B_lda)
	for (ordinal_type i = 0; i < A_nrows; ++i)
	  if (A_j[i] != B_j[i])
	    return false;
      return true;
    }

    Ordinal nrows() const { return nrows_; }
    Ordinal ncols() const { return ncols_; }
    Ordinal lda() const { return nrows_; }
    bool empty() const { return nrows() == 0 || ncols() == 0; }

    Scalar* 
    get() 
    { 
      if (A_.size() > 0)
	return &A_[0]; 
      else
	return static_cast< Scalar* > (NULL);
    }
    const Scalar* 
    get() const
    { 
      if (A_.size() > 0)
	return &A_[0]; 
      else
	return static_cast< const Scalar* > (NULL);
    }

    MatView< Ordinal, Scalar > view () {
      return MatView< Ordinal, Scalar >(nrows(), ncols(), get(), lda());
    }

    ConstMatView< Ordinal, Scalar > const_view () const {
      return ConstMatView< Ordinal, Scalar >(nrows(), ncols(), 
					     (const Scalar*) get(), lda());
    }

    /// Change the dimensions of the matrix.  Reallocate if necessary.
    /// Existing data in the matrix is invalidated.
    ///
    /// \param num_rows [in] New number of rows in the matrix
    /// \param num_cols [in] New number of columns in the matrix
    void
    reshape (const Ordinal num_rows, const Ordinal num_cols)
    {
      if (num_rows == nrows() && num_cols == ncols())
	return; // no need to reallocate or do anything else

      const size_t alloc_size = verified_alloc_size (num_rows, num_cols);
      nrows_ = num_rows;
      ncols_ = num_cols;
      A_.resize (alloc_size);
    }

  private:
    Ordinal nrows_, ncols_;
    std::vector< Scalar > A_;
  };

} // namespace TSQR

#endif // __TSQR_Tsqr_Matrix_hpp

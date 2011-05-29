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

#ifndef __TSQR_Tsqr_Mgs_hpp
#define __TSQR_Tsqr_Mgs_hpp

#include <algorithm>
#include <cassert>
#include <cmath>
#include <utility> // std::pair

#include <Tsqr_MessengerBase.hpp>
#include <Tsqr_ScalarTraits.hpp>
#include <Tsqr_Util.hpp>

#include <Teuchos_RCP.hpp>

// #define MGS_DEBUG 1
#ifdef MGS_DEBUG
#  include <iostream>
using std::cerr;
using std::endl;
#endif // MGS_DEBUG

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {

  template< class LocalOrdinal, class Scalar >
  class MGS {
  public:
    typedef Scalar scalar_type;
    typedef LocalOrdinal ordinal_type;
    typedef typename ScalarTraits<Scalar>::magnitude_type magnitude_type;

    MGS (const Teuchos::RCP< MessengerBase< Scalar > >& messenger) : 
      messenger_ (messenger) {}

    /// Whether or not the R factor from the QR factorization has a
    /// nonnegative diagonal.
    bool QR_produces_R_factor_with_nonnegative_diagonal () const {
      return true;
    }
    
    void 
    mgs (const LocalOrdinal nrows_local, 
	 const LocalOrdinal ncols, 
	 Scalar A_local[], 
	 const LocalOrdinal lda_local,
	 Scalar R[],
	 const LocalOrdinal ldr);

  private:
    Teuchos::RCP< MessengerBase< Scalar > > messenger_;
  };

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

  namespace details {

    template< class LocalOrdinal, class Scalar >
    class MgsOps {
    public:
      MgsOps (const Teuchos::RCP< MessengerBase< Scalar > >& messenger) : 
	messenger_ (messenger) {}

      void
      axpy (const LocalOrdinal nrows_local,
	    const Scalar alpha,
	    const Scalar x_local[],
	    Scalar y_local[]) const
      {
	for (LocalOrdinal i = 0; i < nrows_local; ++i)
	  y_local[i] = y_local[i] + alpha * x_local[i];
      }

      void
      scale (const LocalOrdinal nrows_local, 
	     Scalar x_local[], 
	     const Scalar denom) const
      {
	for (LocalOrdinal i = 0; i < nrows_local; ++i)
	  x_local[i] = x_local[i] / denom;
      }

      /// $y^* \cdot x$: conjugate transpose when Scalar is complex,
      /// else regular transpose.
      Scalar
      dot (const LocalOrdinal nrows_local, 
	   const Scalar x_local[], 
	   const Scalar y_local[])
      {
	Scalar local_result (0);

#ifdef MGS_DEBUG
	// for (LocalOrdinal k = 0; k != nrows_local; ++k)
	//   cerr << "(x[" << k << "], y[" << k << "]) = (" << x_local[k] << "," << y_local[k] << ")" << " ";
	//   cerr << endl;
#endif // MGS_DEBUG

	for (LocalOrdinal i = 0; i < nrows_local; ++i)
	  local_result += x_local[i] * ScalarTraits< double >::conj (y_local[i]);

#ifdef MGS_DEBUG
	  // cerr << "-- Final value on this proc = " << local_result << endl;
#endif // MGS_DEBUG

	// FIXME (mfh 23 Apr 2010) Does MPI_SUM do the right thing for
	// complex or otherwise general MPI data types?  Perhaps an MPI_Op
	// should belong in the MessengerBase...
	return messenger_->globalSum (local_result);
      }

      typename ScalarTraits< Scalar >::magnitude_type 
      norm2 (const LocalOrdinal nrows_local, 
	     const Scalar x_local[])
      {
	Scalar local_result (0);

	// Doing the right thing in the complex case requires taking
	// an absolute value.  We want to avoid this additional cost
	// in the real case, which is why we check is_complex.
	if (ScalarTraits< Scalar >::is_complex) 
	  {
	    for (LocalOrdinal i = 0; i < nrows_local; ++i)
	      {
		const Scalar xi = ScalarTraits< Scalar >::abs (x_local[i]);
		local_result += xi * xi;
	      }
	  }
	else
	  {
	    for (LocalOrdinal i = 0; i < nrows_local; ++i)
	      {
		const Scalar xi = x_local[i];
		local_result += xi * xi;
	      }
	  }
	const Scalar global_result = messenger_->globalSum (local_result);
	// sqrt doesn't make sense if the type of Scalar is complex,
	// even if the imaginary part of global_result is zero.
	return sqrt (ScalarTraits< Scalar >::abs (global_result));
      }

      Scalar
      project (const LocalOrdinal nrows_local, 
	       const Scalar q_local[], 
	       Scalar v_local[])
      {
	const Scalar coeff = this->dot (nrows_local, v_local, q_local);
	this->axpy (nrows_local, -coeff, q_local, v_local);
	return coeff;
      }

    private:
      Teuchos::RCP< MessengerBase< Scalar > > messenger_;
    };
  } // namespace details

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

  template< class LocalOrdinal, class Scalar >
  void
  MGS< LocalOrdinal, Scalar >::mgs (const LocalOrdinal nrows_local, 
				    const LocalOrdinal ncols, 
				    Scalar A_local[], 
				    const LocalOrdinal lda_local,
				    Scalar R[],
				    const LocalOrdinal ldr)
  {
    details::MgsOps< LocalOrdinal, Scalar > ops (messenger_);
    
    for (LocalOrdinal j = 0; j < ncols; ++j)
      {
	Scalar* const v = &A_local[j*lda_local];
	for (LocalOrdinal i = 0; i < j; ++i)
	  {
	    const Scalar* const q = &A_local[i*lda_local];
	    R[i + j*ldr] = ops.project (nrows_local, q, v);
#ifdef MGS_DEBUG
	    if (my_rank == 0)
	      cerr << "(i,j) = (" << i << "," << j << "): coeff = " << R[i + j*ldr] << endl;
#endif // MGS_DEBUG
	  }
	const magnitude_type denom = ops.norm2 (nrows_local, v);
#ifdef MGS_DEBUG
	  if (my_rank == 0)
	    cerr << "j = " << j << ": denom = " << denom << endl;
#endif // MGS_DEBUG

	// FIXME (mfh 29 Apr 2010)
	//
	// NOTE IMPLICIT CAST.  This should work for complex numbers.
	// If it doesn't work for your Scalar data type, it means that
	// you need a different data type for the diagonal elements of
	// the R factor, than you need for the other elements.  This
	// is unlikely if we're comparing MGS against a Householder QR
	// factorization; I don't really understand how the latter
	// would work (not that it couldn't be given a sensible
	// interpretation) in the case of Scalars that aren't plain
	// old real or complex numbers.
	R[j + j*ldr] = Scalar (denom);
	ops.scale (nrows_local, v, denom);
      }
  }

} // namespace TSQR

#endif // __TSQR_Tsqr_Mgs_hpp

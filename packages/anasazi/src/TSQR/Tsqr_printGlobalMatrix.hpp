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

#ifndef __Tsqr_printGlobalMatrix_hpp
#define __Tsqr_printGlobalMatrix_hpp

#include "Tsqr_MessengerBase.hpp"
#include "Tsqr_Util.hpp"
#include "Tsqr_Matrix.hpp"

#include <limits>
#include <ostream>
#include <stdexcept>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {

  /// \fn printGlobalMatrix
  /// \brief Print a dense matrix distributed among all MPI processes
  ///
  template< class ConstMatrixViewType >
  void
  printGlobalMatrix (std::ostream& out,
		     const ConstMatrixViewType& A_local,
		     MessengerBase< typename ConstMatrixViewType::scalar_type >* const scalarComm,
		     MessengerBase< typename ConstMatrixViewType::ordinal_type >* const ordinalComm)
    {
      typedef typename ConstMatrixViewType::ordinal_type LocalOrdinal;
      typedef typename ConstMatrixViewType::scalar_type Scalar;
      using std::endl;

      const int myRank = scalarComm->rank ();
      const int nprocs = scalarComm->size ();
      const LocalOrdinal nrowsLocal = A_local.nrows();
      const LocalOrdinal ncols = A_local.ncols();
      const Scalar quiet_NaN = std::numeric_limits< Scalar >::quiet_NaN();

      if (myRank == 0)
	{
	  // Print the remote matrix data
	  // out << "Processor " << my_rank << ":" << endl;
	  print_local_matrix (out, A_local.nrows(), A_local.ncols(), 
			      A_local.get(), A_local.lda());

	  // Space for remote matrix data.  Other processors are allowed
	  // to have different nrows_local values; we make space as
	  // necessary.
	  Matrix< LocalOrdinal, Scalar > A_remote (nrowsLocal, ncols, quiet_NaN);

	  // Loop through all the other processors in order.
	  // Fetch their matrix data and print it.
	  for (int srcProc = 1; srcProc < nprocs; ++srcProc)
	    {
	      // Get processor proc's local matrix dimensions
	      LocalOrdinal dims[2];
	      ordinalComm->recv (&dims[0], 2, srcProc, 0);

	      // Make space for the remote matrix data
	      if (std::numeric_limits< LocalOrdinal >::is_signed)
		{
		  if (dims[0] <= 0 || dims[1] <= 0)
		    throw std::runtime_error ("Invalid dimensions of remote matrix");
		}
	      else
		{
		  if (dims[0] == 0 || dims[1] == 0)
		    throw std::runtime_error ("Invalid dimensions of remote matrix");
		}
	      A_remote.reshape (dims[0], dims[1]);

	      // Receive the remote matrix data, which we assume is
	      // stored contiguously.
	      scalarComm->recv (A_remote.get(), dims[0]*dims[1], srcProc, 0);

	      // Print the remote matrix data
	      // out << "Processor " << proc << ":" << endl;
	      print_local_matrix (out, dims[0], dims[0], A_remote.get(), A_remote.lda());
	    }
	}
      else
	{
	  // Send my local matrix dimensions to proc 0.
	  int rootProc = 0;
	  LocalOrdinal dims[2];

	  dims[0] = nrowsLocal;
	  dims[1] = ncols;
	  ordinalComm->send (dims, 2, rootProc, 0);

	  // Create a (contiguous) buffer and copy the data into it.
	  Matrix< LocalOrdinal, Scalar > A_buf (nrowsLocal, ncols, quiet_NaN);
	  A_buf.copy (A_local);

	  // Send the actual data to proc 0.
	  scalarComm->send (A_buf.get(), nrowsLocal*ncols, rootProc, 0);
	}
      scalarComm->barrier ();
    }

} // namespace TSQR

#endif // __Tsqr_printGlobalMatrix_hpp

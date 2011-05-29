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

#ifndef __TSQR_TBB_ExplicitQTask_hpp
#define __TSQR_TBB_ExplicitQTask_hpp

#include <tbb/task.h>
#include <TbbTsqr_Partitioner.hpp>
#include <Tsqr_SequentialTsqr.hpp>

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

namespace TSQR {
  namespace TBB {
    
    template< class LocalOrdinal, class Scalar >
    class ExplicitQTask : public tbb::task {
    private:
      typedef MatView< LocalOrdinal, Scalar > mat_view;
      typedef ConstMatView< LocalOrdinal, Scalar > const_mat_view;
      typedef std::pair< mat_view, mat_view > split_t;
      typedef std::pair< const_mat_view, const_mat_view > const_split_t;

    public:
      ExplicitQTask (const size_t P_first__, 
		     const size_t P_last__,
		     MatView< LocalOrdinal, Scalar > Q_out,
		     const SequentialTsqr< LocalOrdinal, Scalar >& seq,
		     const bool contiguous_cache_blocks) :
	P_first_ (P_first__), P_last_ (P_last__), Q_out_ (Q_out),
	seq_ (seq), contiguous_cache_blocks_ (contiguous_cache_blocks)
      {}

      tbb::task* execute () {
	if (P_first_ > P_last_ || Q_out_.empty())
	  return NULL;
	else if (P_first_ == P_last_)
	  {
	    // Fill my partition with zeros.
	    seq_.fill_with_zeros (Q_out_.nrows(), Q_out_.ncols(), Q_out_.get(), 
				  Q_out_.lda(), contiguous_cache_blocks_);

	    // If our partition is the first (topmost), fill it with
	    // the first Q_out.ncols() columns of the identity matrix.
	    if (P_first_ == 0)
	      {
		// Fetch the topmost cache block of my partition.  Its
		// leading dimension should be set correctly by
		// top_block().
		mat_view Q_out_top = seq_.top_block (Q_out_, contiguous_cache_blocks_);
		// Set the top block of Q_out to the first ncols
		// columns of the identity matrix.
		for (LocalOrdinal j = 0; j < Q_out_top.ncols(); ++j)
		  Q_out_top(j,j) = Scalar(1);
	      }
	    return NULL;
	  }
	else
	  {
	    // "c": continuation task
	    tbb::empty_task& c = *new( allocate_continuation() ) tbb::empty_task;

	    // Recurse on two intervals: [P_first, P_mid] and [P_mid+1, P_last]
	    const size_t P_mid = (P_first_ + P_last_) / 2;
	    split_t Q_split = 
	      partitioner_.split (Q_out_, P_first_, P_mid, P_last_,
				  contiguous_cache_blocks_);

	    ExplicitQTask& topTask = *new( c.allocate_child() )
	      ExplicitQTask (P_first_, P_mid, Q_split.first, seq_, 
			     contiguous_cache_blocks_);
	    ExplicitQTask& botTask = *new( c.allocate_child() )
	      ExplicitQTask (P_mid+1, P_last_, Q_split.second, seq_, 
			     contiguous_cache_blocks_);

	    // Set reference count of parent (in this case, the
	    // continuation task) to 2 (since 2 children -- no
	    // additional task since no waiting).
	    c.set_ref_count (2);
	    c.spawn (botTask);
	    return &topTask; // scheduler bypass optimization
	  }
      }

    private:
      size_t P_first_, P_last_;
      mat_view Q_out_;
      SequentialTsqr< LocalOrdinal, Scalar > seq_;
      Partitioner< LocalOrdinal, Scalar > partitioner_;
      bool contiguous_cache_blocks_;
    };

  } // namespace TBB
} // namespace TSQR


#endif // __TSQR_TBB_ExplicitQTask_hpp

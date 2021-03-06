// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
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

#ifndef THYRA_TPETRA_MULTIVECTOR_HPP
#define THYRA_TPETRA_MULTIVECTOR_HPP

#include "Thyra_TpetraMultiVector_decl.hpp"
#include "Thyra_TpetraVectorSpace.hpp"
#include "Thyra_TpetraVector.hpp"
#include "Teuchos_Assert.hpp"


namespace Thyra {


// Constructors/initializers/accessors


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::TpetraMultiVector()
{}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::initialize(
  const RCP<const TpetraVectorSpace<Scalar,LocalOrdinal,GlobalOrdinal,Node> > &tpetraVectorSpace,
  const RCP<const ScalarProdVectorSpaceBase<Scalar> > &domainSpace,
  const RCP<Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> > &tpetraMultiVector
  )
{
  initializeImpl(tpetraVectorSpace, domainSpace, tpetraMultiVector);
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::constInitialize(
  const RCP<const TpetraVectorSpace<Scalar,LocalOrdinal,GlobalOrdinal,Node> > &tpetraVectorSpace,
  const RCP<const ScalarProdVectorSpaceBase<Scalar> > &domainSpace,
  const RCP<const Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> > &tpetraMultiVector
  )
{
  initializeImpl(tpetraVectorSpace, domainSpace, tpetraMultiVector);
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::getTpetraMultiVector()
{
  return tpetraMultiVector_.getNonconstObj();
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const Tpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::getConstTpetraMultiVector() const
{
  return tpetraMultiVector_;
}


// Overridden public functions form MultiVectorAdapterBase


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP< const ScalarProdVectorSpaceBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::domainScalarProdVecSpc() const
{
  return domainSpace_;
}


// Overridden public functions from SpmdMultiVectorBase


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const SpmdVectorSpaceBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::spmdSpace() const
{
  return tpetraVectorSpace_;
}


// Overridden protected functions from MultiVectorBase


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const VectorBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::colImpl(Ordinal j) const
{
#ifdef TEUCHOS_DEBUG
  TEUCHOS_ASSERT_IN_RANGE_UPPER_EXCLUSIVE(j, 0, this->domain()->dim());
#endif
  return constTpetraVector<Scalar>(
    tpetraVectorSpace_,
    tpetraMultiVector_->getVector(j)
    );
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<VectorBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::nonconstColImpl(Ordinal j)
{
#ifdef TEUCHOS_DEBUG
  TEUCHOS_ASSERT_IN_RANGE_UPPER_EXCLUSIVE(j, 0, this->domain()->dim());
#endif
  return tpetraVector<Scalar>(
    tpetraVectorSpace_,
    tpetraMultiVector_.getNonconstObj()->getVectorNonConst(j)
    );
}


/* ToDo: Implement these?


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<MultiVectorBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::nonconstContigSubViewImpl(
  const Range1D& col_rng_in
  )
{
#ifdef THYRA_DEFAULT_SPMD_MULTI_VECTOR_VERBOSE_TO_ERROR_OUT
  std::cerr << "\nSpmdMultiVectorStd<Scalar>::subView() called!\n";
#endif
  const Range1D colRng = this->validateColRange(col_rng_in);
  return Teuchos::rcp(
    new TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>(
      spmdRangeSpace_,
      Teuchos::rcp_dynamic_cast<const ScalarProdVectorSpaceBase<Scalar> >(
        spmdRangeSpace_->smallVecSpcFcty()->createVecSpc(colRng.size())
        ,true
        ),
      localValues_.persistingView(colRng.lbound()*leadingDim_,colRng.size()*spmdRangeSpace_->localSubDim()),
      leadingDim_
      )
    );
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<const MultiVectorBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::nonContigSubViewImpl(
  const ArrayView<const int> &cols
  ) const
{
  THYRA_DEBUG_ASSERT_MV_COLS("nonContigSubViewImpl(cols)", cols);
  const int numCols = cols.size();
  const ArrayRCP<Scalar> localValuesView = createContiguousCopy(cols);
  return defaultSpmdMultiVector<Scalar>(
    spmdRangeSpace_,
    createSmallScalarProdVectorSpaceBase<Scalar>(spmdRangeSpace_, numCols),
    localValuesView
    );
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
RCP<MultiVectorBase<Scalar> >
TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::nonconstNonContigSubViewImpl(
  const ArrayView<const int> &cols )
{
  THYRA_DEBUG_ASSERT_MV_COLS("nonContigSubViewImpl(cols)", cols);
  const int numCols = cols.size();
  const ArrayRCP<Scalar> localValuesView = createContiguousCopy(cols);
  const Ordinal localSubDim = spmdRangeSpace_->localSubDim();
  RCP<CopyBackSpmdMultiVectorEntries<Scalar> > copyBackView =
    copyBackSpmdMultiVectorEntries<Scalar>(cols, localValuesView.getConst(),
      localSubDim, localValues_.create_weak(), leadingDim_);
  return Teuchos::rcpWithEmbeddedObjPreDestroy(
    new TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>(
      spmdRangeSpace_,
      createSmallScalarProdVectorSpaceBase<Scalar>(spmdRangeSpace_, numCols),
      localValuesView),
    copyBackView
    );
}

*/


// Overridden protected members from SpmdMultiVectorBase


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::getNonconstLocalDataImpl(
  const Ptr<ArrayRCP<Scalar> > &localValues, const Ptr<Ordinal> &leadingDim
  )
{
  *localValues = tpetraMultiVector_.getNonconstObj()->get1dViewNonConst();
  *leadingDim = tpetraMultiVector_->getStride();
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
void TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::getLocalDataImpl(
  const Ptr<ArrayRCP<const Scalar> > &localValues, const Ptr<Ordinal> &leadingDim
  ) const
{
  *localValues = tpetraMultiVector_->get1dView();
  *leadingDim = tpetraMultiVector_->getStride();
}


// private


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node>
template<class TpetraMultiVector_t>
void TpetraMultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node>::initializeImpl(
  const RCP<const TpetraVectorSpace<Scalar,LocalOrdinal,GlobalOrdinal,Node> > &tpetraVectorSpace,
  const RCP<const ScalarProdVectorSpaceBase<Scalar> > &domainSpace,
  const RCP<TpetraMultiVector_t> &tpetraMultiVector
  )
{
#ifdef THYRA_DEBUG
  TEUCHOS_ASSERT(nonnull(tpetraVectorSpace));
  TEUCHOS_ASSERT(nonnull(domainSpace));
  TEUCHOS_ASSERT(nonnull(tpetraMultiVector));
  // ToDo: Check to make sure that tpetraMultiVector is compatible with
  // tpetraVectorSpace.
#endif
  tpetraVectorSpace_ = tpetraVectorSpace;
  domainSpace_ = domainSpace;
  tpetraMultiVector_.initialize(tpetraMultiVector);
  this->updateSpmdSpace();
}



} // end namespace Thyra


#endif // THYRA_TPETRA_MULTIVECTOR_HPP

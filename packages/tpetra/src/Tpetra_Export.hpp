// @HEADER
// ***********************************************************************
// 
//          Tpetra: Templated Linear Algebra Services Package
//                 Copyright (2008) Sandia Corporation
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

#ifndef TPETRA_EXPORT_HPP
#define TPETRA_EXPORT_HPP

#include <Kokkos_DefaultNode.hpp>
#include <Teuchos_Describable.hpp>
#include <Teuchos_as.hpp>
#include "Tpetra_Map.hpp"
#include "Tpetra_Util.hpp"
#include "Tpetra_ImportExportData.hpp"
#include <iterator>

namespace Tpetra {

  //! \brief This class builds an object containing information necesary for efficiently exporting entries off-processor.
  /*! Export is used to construct a communication plan that can be called repeatedly by computational
      classes to efficiently export entries to other nodes.
      For example, an exporter is used when we start out with a multiple-ownership distribution,
      and we want to merge that into a uniquely-owned distribution.

      This class currently has one constructor, taking two Map objects
      specifying the distributions of the distributed objects on which the Export class will operate.

      This class is templated on \c LocalOrdinal and \c GlobalOrdinal. 
      The \c GlobalOrdinal type, if omitted, defaults to the \c LocalOrdinal type.
   */

  template <class LocalOrdinal, class GlobalOrdinal = LocalOrdinal, class Node = Kokkos::DefaultNode::DefaultNodeType>
  class Export: public Teuchos::Describable {

  public:

    //! @name Constructor/Destructor Methods
    //@{ 

    //! Constructs a Export object from the source and target Map.
    Export(const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & source,  
           const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & target);

    //! copy constructor. 
    Export(const Export<LocalOrdinal,GlobalOrdinal,Node> & rhs);

    //! destructor.
    ~Export();

    //@}

    //! @name Export Attribute Methods
    //@{ 

    //! Returns the number of entries that are identical between the source and target Maps, up to the first different ID.
    size_t getNumSameIDs() const;

    //! Returns the number of entries that are local to the calling image, but not part of the first getNumSameIDs() entries.
    size_t getNumPermuteIDs() const;

    //! List of entries in the source Map that are permuted. (non-persisting view)
    Teuchos::ArrayView<const LocalOrdinal> getPermuteFromLIDs() const;

    //! List of entries in the target Map that are permuted. (non-persisting view)
    Teuchos::ArrayView<const LocalOrdinal> getPermuteToLIDs() const;

    //! Returns the number of entries that are not on the calling image.
    size_t getNumRemoteIDs() const;

    //! List of entries in the target Map that are coming from other images. (non-persisting view)
    Teuchos::ArrayView<const LocalOrdinal> getRemoteLIDs() const;

    //! Returns the number of entries that must be sent by the calling image to other images.
    size_t getNumExportIDs() const;

    //! List of entries in the source Map that will be sent to other images. (non-persisting view)
    Teuchos::ArrayView<const LocalOrdinal> getExportLIDs() const;

    //! List of images to which entries will be sent, getExportLIDs() [i] will be sent to image getExportImageIDs() [i]. (non-persisting view)
    Teuchos::ArrayView<const int> getExportImageIDs() const;

    //! Returns the Source Map used to construct this exporter.
    const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & getSourceMap() const;

    //! Returns the Target Map used to construct this exporter.
    const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & getTargetMap() const;

    Distributor & getDistributor() const;

    //! Assignment operator
    Export<LocalOrdinal,GlobalOrdinal,Node>& operator = (const Export<LocalOrdinal,GlobalOrdinal,Node> & Source);

    //@}

    //! @name I/O Methods
    //@{ 

    //! Print method
    virtual void print(std::ostream& os) const;

    //@}

  private:

    Teuchos::RCP<ImportExportData<LocalOrdinal,GlobalOrdinal,Node> > ExportData_;

    // subfunctions used by constructor
    //==============================================================================
    // sets up numSameIDs_, numPermuteIDs_, and numExportIDs_
    // these variables are already initialized to 0 by the ImportExportData ctr.
    // also sets up permuteToLIDs_, permuteFromLIDs_, exportGIDs_, and exportLIDs_
    void setupSamePermuteExport();
    void setupRemote();
  };

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Export<LocalOrdinal,GlobalOrdinal,Node>::Export(const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & source,   
                                                  const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & target) {
    ExportData_ = Teuchos::rcp(new ImportExportData<LocalOrdinal,GlobalOrdinal,Node>(source, target));
    // call subfunctions
    setupSamePermuteExport();
    if (source->isDistributed()) {
      setupRemote();
    }
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Export<LocalOrdinal,GlobalOrdinal,Node>::Export(const Export<LocalOrdinal,GlobalOrdinal,Node> & rhs)
  : ExportData_(rhs.ExportData_)
  {}

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Export<LocalOrdinal,GlobalOrdinal,Node>::~Export() 
  {}

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  size_t Export<LocalOrdinal,GlobalOrdinal,Node>::getNumSameIDs() const {
    return ExportData_->numSameIDs_;
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  size_t Export<LocalOrdinal,GlobalOrdinal,Node>::getNumPermuteIDs() const {
    return ExportData_->permuteFromLIDs_.size();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::ArrayView<const LocalOrdinal> 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getPermuteFromLIDs() const {
    return ExportData_->permuteFromLIDs_();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::ArrayView<const LocalOrdinal>
  Export<LocalOrdinal,GlobalOrdinal,Node>::getPermuteToLIDs() const {
    return ExportData_->permuteToLIDs_();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  size_t Export<LocalOrdinal,GlobalOrdinal,Node>::getNumRemoteIDs() const {
    return ExportData_->remoteLIDs_.size();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::ArrayView<const LocalOrdinal> 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getRemoteLIDs() const {
    return ExportData_->remoteLIDs_();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  size_t Export<LocalOrdinal,GlobalOrdinal,Node>::getNumExportIDs() const {
    return ExportData_->exportLIDs_.size();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::ArrayView<const LocalOrdinal> 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getExportLIDs() const {
    return ExportData_->exportLIDs_();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Teuchos::ArrayView<const int> 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getExportImageIDs() const {
    return ExportData_->exportImageIDs_();
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getSourceMap() const {
    return ExportData_->source_;
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  const Teuchos::RCP<const Map<LocalOrdinal,GlobalOrdinal,Node> > & 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getTargetMap() const {
    return ExportData_->target_;
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Distributor & 
  Export<LocalOrdinal,GlobalOrdinal,Node>::getDistributor() const { return ExportData_->distributor_;
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  Export<LocalOrdinal,GlobalOrdinal,Node>& 
  Export<LocalOrdinal,GlobalOrdinal,Node>::operator=(const Export<LocalOrdinal,GlobalOrdinal,Node> & source) {
    ExportData_ = source.ExportData_;
    return *this;
  }

  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  void Export<LocalOrdinal,GlobalOrdinal,Node>::print(std::ostream& os) const {
    using std::endl;
    Teuchos::ArrayView<const LocalOrdinal> av;
    Teuchos::ArrayView<const int> avi;
    const Teuchos::RCP<const Teuchos::Comm<int> > & comm = getSourceMap()->getComm();
    const int myImageID = comm->getRank();
    const int numImages = comm->getSize();
    for (int imageCtr = 0; imageCtr < numImages; ++imageCtr) {
      if(myImageID == imageCtr) {
        os << endl;
        if(myImageID == 0) { // this is the root node (only output this info once)
          os << "Export Data Members:" << endl;
        }
        os << "Image ID       : " << myImageID << endl;
        os << "permuteFromLIDs: {"; av = getPermuteFromLIDs(); std::copy(av.begin(),av.end(),std::ostream_iterator<LocalOrdinal>(os," ")); os << " }" << endl;
        os << "permuteToLIDs  : {"; av = getPermuteToLIDs();   std::copy(av.begin(),av.end(),std::ostream_iterator<LocalOrdinal>(os," ")); os << " }" << endl;
        os << "remoteLIDs     : {"; av = getRemoteLIDs();      std::copy(av.begin(),av.end(),std::ostream_iterator<LocalOrdinal>(os," ")); os << " }" << endl;
        os << "exportLIDs     : {"; av = getExportLIDs();      std::copy(av.begin(),av.end(),std::ostream_iterator<LocalOrdinal>(os," ")); os << " }" << endl;
        os << "exportImageIDs : {"; avi = getExportImageIDs();  std::copy(avi.begin(),avi.end(),std::ostream_iterator<int>(os," ")); os << " }" << endl;
        os << "numSameIDs     : " << getNumSameIDs() << endl;
        os << "numPermuteIDs  : " << getNumPermuteIDs() << endl;
        os << "numRemoteIDs   : " << getNumRemoteIDs() << endl;
        os << "numExportIDs   : " << getNumExportIDs() << endl;
      }
      // Do a few global ops to give I/O a chance to complete
      comm->barrier();
      comm->barrier();
      comm->barrier();
    }
    if (myImageID == 0) {
      os << "\nSource Map: " << endl; 
    }
    os << *getSourceMap();
    if (myImageID == 0) {
      os << "\nTarget Map: " << endl; 
    }
    os << *getTargetMap();
  }


  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  void Export<LocalOrdinal,GlobalOrdinal,Node>::setupSamePermuteExport() {
    const Map<LocalOrdinal,GlobalOrdinal,Node> & source = *getSourceMap();
    const Map<LocalOrdinal,GlobalOrdinal,Node> & target = *getTargetMap();
    Teuchos::ArrayView<const GlobalOrdinal> sourceGIDs = source.getNodeElementList();
    Teuchos::ArrayView<const GlobalOrdinal> targetGIDs = target.getNodeElementList();

    // -- compute numSameIDs_ ---
    // go through GID lists of source and target. if the ith GID on both is the same, 
    // increment numSameIDs_ and try the next. as soon as you come to a pair that don't
    // match, give up.
    typename Teuchos::ArrayView<const GlobalOrdinal>::iterator sourceIter = sourceGIDs.begin(),
                                                               targetIter = targetGIDs.begin();
    while( sourceIter != sourceGIDs.end() && targetIter != targetGIDs.end() && *sourceIter == *targetIter ) {
      ++ExportData_->numSameIDs_;
      ++sourceIter;
      ++targetIter;
    }
    // sourceIter should now point to the GID of the first non-same entry or at the end of targetGIDs

    // -- compute numPermuteIDs --
    // -- fill permuteToLIDs_, permuteFromLIDs_ --
    // go through remaining entries in sourceGIDs. if target owns that GID, 
    // increment numPermuteIDs_, and add entries to permuteToLIDs_ and permuteFromLIDs_.
    // otherwise increment numExportIDs_ and add entries to exportLIDs_ and exportGIDs_.
    for(; sourceIter != sourceGIDs.end(); ++sourceIter) {
      if(target.isNodeGlobalElement(*sourceIter)) {
        // both source and target list this GID (*targetIter)
        // determine the LIDs for this GID on both Maps and add them to the permutation lists
        ExportData_->permuteToLIDs_.push_back(  target.getLocalElement(*sourceIter));
        ExportData_->permuteFromLIDs_.push_back(source.getLocalElement(*sourceIter));
      }
      else {
        ExportData_->exportGIDs_.push_back(*sourceIter);
      }
    }

    TEST_FOR_EXCEPTION( (getNumExportIDs() > 0) && (!source.isDistributed()), std::runtime_error, 
        Teuchos::typeName(*this) << "::setupSamePermuteExport(): Source has export LIDs but Source is not distributed globally.");

    // -- compute exportImageIDs_ --
    // get list of images that own the GIDs in exportGIDs_ (in the target Map)
    ExportData_->exportImageIDs_ = Teuchos::arcp<int>(ExportData_->exportGIDs_.size());
    ExportData_->exportLIDs_     = Teuchos::arcp<LocalOrdinal>(ExportData_->exportGIDs_.size());
    {
      typename Teuchos::ArrayRCP<LocalOrdinal>::iterator liditer = ExportData_->exportLIDs_.begin();
      typename Teuchos::Array<GlobalOrdinal>::iterator    giditer = ExportData_->exportGIDs_.begin();
      for (; giditer != ExportData_->exportGIDs_.end(); ++liditer, ++giditer) {
        *liditer = source.getLocalElement(*giditer);
      }
    }
    TEST_FOR_EXCEPTION( target.getRemoteIndexList(ExportData_->exportGIDs_(), ExportData_->exportImageIDs_()) == IDNotPresent,
        std::runtime_error, "Tpetra::Export::setupSamePermuteExport(): Source has GIDs not found in Target.");
  }


  template <class LocalOrdinal, class GlobalOrdinal, class Node>
  void Export<LocalOrdinal,GlobalOrdinal,Node>::setupRemote() {
    const Map<LocalOrdinal,GlobalOrdinal,Node> & target = *getTargetMap();

    // make sure export IDs are ordered by image
    // sort exportImageIDs_ in ascending order,
    // and apply the same permutation to exportGIDs_ and exportLIDs_.
    sort3(ExportData_->exportImageIDs_.begin(), ExportData_->exportImageIDs_.end(), ExportData_->exportGIDs_.begin(), ExportData_->exportLIDs_.begin());

    // Construct list of entries that calling image needs to send as a result
    // of everyone asking for what it needs to receive.
    size_t numRemoteIDs;
    numRemoteIDs = ExportData_->distributor_.createFromSends(ExportData_->exportImageIDs_());

    // Use comm plan with ExportGIDs to find out who is sending to us and
    // get proper ordering of GIDs for remote entries 
    // (these will be converted to LIDs when done).
    Teuchos::Array<GlobalOrdinal> remoteGIDs(numRemoteIDs);
    ExportData_->distributor_.doPostsAndWaits(ExportData_->exportGIDs_().getConst(),1,remoteGIDs());

    // Remote IDs come in as GIDs, convert to LIDs
    ExportData_->remoteLIDs_.resize(numRemoteIDs);
    {
      typename Teuchos::Array<GlobalOrdinal>::const_iterator i = remoteGIDs.begin();
      typename Teuchos::Array<LocalOrdinal>::iterator       j = ExportData_->remoteLIDs_.begin();
      while (i != remoteGIDs.end()) 
      {
        *j++ = target.getLocalElement(*i++);
      }
    }
  }

} // namespace Tpetra

#endif // TPETRA_EXPORT_HPP

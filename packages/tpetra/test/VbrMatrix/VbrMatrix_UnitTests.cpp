#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_Array.hpp>
#include <Teuchos_as.hpp>
#include <Teuchos_Tuple.hpp>
#include <Teuchos_VerboseObject.hpp>
#include <Teuchos_oblackholestream.hpp>
#include <Teuchos_FancyOStream.hpp>
#include <Teuchos_ScalarTraits.hpp>
#include <Teuchos_OrdinalTraits.hpp>
#include <Teuchos_TypeTraits.hpp>

#include "Tpetra_ConfigDefs.hpp"
#include "Tpetra_DefaultPlatform.hpp"
#include "Tpetra_BlockMultiVector.hpp"
#include "Tpetra_VbrMatrix.hpp"

#include "Kokkos_SerialNode.hpp"
#ifdef HAVE_KOKKOS_TBB
#include "Kokkos_TBBNode.hpp"
#endif
#ifdef HAVE_KOKKOS_THREADPOOL
#include "Kokkos_TPINode.hpp"
#endif
#ifdef HAVE_KOKKOS_THRUST
#include "Kokkos_ThrustGPUNode.hpp"
#endif


// TODO: add test where some nodes have zero rows
// TODO: add test where non-"zero" graph is used to build matrix; if no values are added to matrix, the operator effect should be zero. This tests that matrix values are initialized properly.
// TODO: add test where dynamic profile initially has no allocation, then entries are added. this will test new view functionality.

namespace Teuchos {
  template <>
    ScalarTraits<int>::magnitudeType
    relErr( const int &s1, const int &s2 )
    {
      typedef ScalarTraits<int> ST;
      return ST::magnitude(s1-s2);
    }

  template <>
    ScalarTraits<char>::magnitudeType
    relErr( const char &s1, const char &s2 )
    {
      typedef ScalarTraits<char> ST;
      return ST::magnitude(s1-s2);
    }
}

namespace {

  using std::endl;
  using std::swap;

  using std::string;

  using Teuchos::TypeTraits::is_same;
  using Teuchos::FancyOStream;
  using Teuchos::RCP;
  using Teuchos::ArrayRCP;
  using Teuchos::rcp;
  using Teuchos::outArg;
  using Teuchos::arcpClone;
  using Teuchos::arrayView;
  using Teuchos::broadcast;
  using Teuchos::OrdinalTraits;
  using Teuchos::ScalarTraits;
  using Teuchos::Comm;
  using Teuchos::Array;
  using Teuchos::ArrayView;
  using Teuchos::tuple;
  using Teuchos::null;
  using Teuchos::VERB_NONE;
  using Teuchos::VERB_LOW;
  using Teuchos::VERB_MEDIUM;
  using Teuchos::VERB_HIGH;
  using Teuchos::VERB_EXTREME;
  using Teuchos::ETransp;
  using Teuchos::NO_TRANS;
  using Teuchos::TRANS;
  using Teuchos::CONJ_TRANS;
  using Teuchos::EDiag;
  using Teuchos::UNIT_DIAG;
  using Teuchos::NON_UNIT_DIAG;
  using Teuchos::EUplo;
  using Teuchos::UPPER_TRI;
  using Teuchos::LOWER_TRI;

  using Tpetra::BlockMap;
  using Tpetra::BlockMultiVector;
  using Tpetra::BlockCrsGraph;
  using Tpetra::Operator;
  using Tpetra::VbrMatrix;
  using Tpetra::Import;
  using Tpetra::global_size_t;
  using Tpetra::DefaultPlatform;
  using Tpetra::ProfileType;
  using Tpetra::StaticProfile;
  using Tpetra::DynamicProfile;
  using Tpetra::OptimizeOption;
  using Tpetra::DoOptimizeStorage;
  using Tpetra::DoNotOptimizeStorage;
  using Tpetra::GloballyDistributed;
  using Tpetra::INSERT;

  using Kokkos::SerialNode;
  RCP<SerialNode> snode;
#ifdef HAVE_KOKKOS_TBB
  using Kokkos::TBBNode;
  RCP<TBBNode> tbbnode;
#endif
#ifdef HAVE_KOKKOS_THREADPOOL
  using Kokkos::TPINode;
  RCP<TPINode> tpinode;
#endif
#ifdef HAVE_KOKKOS_THRUST
  using Kokkos::ThrustGPUNode;
  RCP<ThrustGPUNode> thrustnode;
#endif

  bool testMpi = true;
  double errorTolSlack = 1e+1;
  string filedir;

#define DYN_ARRAYVIEW_TO_ARRAY(Type, arr, av) \
  { \
    ArrayView<Type> av2 = av; \
    arr.resize(av2.size()); \
    arr.assign(av2.begin(),av2.end()); \
  }

#define STD_TESTS(matrix) \
  { \
    using Teuchos::outArg; \
    RCP<const Comm<int> > STCOMM = matrix.getComm(); \
    ArrayView<const GO> STMYGIDS = matrix.getRowMap()->getNodeElementList(); \
    ArrayRCP<const LO> loview; \
    ArrayRCP<const Scalar> sview; \
    size_t STMAX = 0; \
    for (size_t STR=0; STR < matrix.getNodeNumRows(); ++STR) { \
      const size_t numEntries = matrix.getNumEntriesInLocalRow(STR); \
      TEST_EQUALITY( numEntries, matrix.getNumEntriesInGlobalRow( STMYGIDS[STR] ) ); \
      matrix.getLocalRowView(STR,loview,sview); \
      TEST_EQUALITY( static_cast<size_t>(loview.size()), numEntries ); \
      TEST_EQUALITY( static_cast<size_t>( sview.size()), numEntries ); \
      STMAX = std::max( STMAX, numEntries ); \
    } \
    TEST_EQUALITY( matrix.getNodeMaxNumRowEntries(), STMAX ); \
    global_size_t STGMAX; \
    Teuchos::reduceAll<int,global_size_t>( *STCOMM, Teuchos::REDUCE_MAX, STMAX, outArg(STGMAX) ); \
    TEST_EQUALITY( matrix.getGlobalMaxNumRowEntries(), STGMAX ); \
  }


  TEUCHOS_STATIC_SETUP()
  {
    Teuchos::CommandLineProcessor &clp = Teuchos::UnitTestRepository::getCLP();
    clp.setOption(
        "filedir",&filedir,"Directory of expected matrix files.");
    clp.addOutputSetupOptions(true);
    clp.setOption(
        "test-mpi", "test-serial", &testMpi,
        "Test MPI (if available) or force test of serial.  In a serial build,"
        " this option is ignord and a serial comm is always used." );
    clp.setOption(
        "error-tol-slack", &errorTolSlack,
        "Slack off of machine epsilon used to check test results" );
  }

  RCP<const Comm<int> > getDefaultComm()
  {
    RCP<const Comm<int> > ret;
    if (testMpi) {
      ret = DefaultPlatform::getDefaultPlatform().getComm();
    }
    else {
      ret = rcp(new Teuchos::SerialComm<int>());
    }
    return ret;
  }

  template <class Node>
  RCP<Node> getNode() {
    assert(false);
  }

  template<class Scalar,class Ordinal>
  void zero_lower_triangle(Ordinal N, Teuchos::Array<Scalar>& A)
  {
    Scalar zero = Teuchos::ScalarTraits<Scalar>::zero();
    for(Ordinal r=0; r<N; ++r) {
      for(Ordinal c=0; c<r; ++c) {
        A[c*N+r] = zero;
      }
    }
  }

  template <>
  RCP<SerialNode> getNode<SerialNode>() {
    if (snode == null) {
      Teuchos::ParameterList pl;
      snode = rcp(new SerialNode(pl));
    }
    return snode;
  }

#ifdef HAVE_KOKKOS_TBB
  template <>
  RCP<TBBNode> getNode<TBBNode>() {
    if (tbbnode == null) {
      Teuchos::ParameterList pl;
      pl.set<int>("Num Threads",0);
      tbbnode = rcp(new TBBNode(pl));
    }
    return tbbnode;
  }
#endif

#ifdef HAVE_KOKKOS_THREADPOOL
  template <>
  RCP<TPINode> getNode<TPINode>() {
    if (tpinode == null) {
      Teuchos::ParameterList pl;
      pl.set<int>("Num Threads",0);
      tpinode = rcp(new TPINode(pl));
    }
    return tpinode;
  }
#endif

#ifdef HAVE_KOKKOS_THRUST
  template <>
  RCP<ThrustGPUNode> getNode<ThrustGPUNode>() {
    if (thrustnode == null) {
      Teuchos::ParameterList pl;
      pl.set<int>("Num Threads",0);
      pl.set<int>("Verbose",1);
      thrustnode = rcp(new ThrustGPUNode(pl));
    }
    return thrustnode;
  }
#endif

  //
  // UNIT TESTS
  // 

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, Basic, LO, GO, Scalar, Node )
  {
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 5;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      TEST_EQUALITY(vbr->hasTransposeApply(), false);
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, SetAndGetBlockEntry1, LO, GO, Scalar, Node )
  {
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::SerialDenseMatrix<GO,Scalar> blkEntry(blockSize, blockSize);
          blkEntry.putScalar(row+col+1);
          vbr->setGlobalBlockEntry(row, col, blkEntry);
        }
      }
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          LO numPtRows, numPtCols;
          Teuchos::ArrayRCP<const Scalar> blockEntry;
          vbr->getGlobalBlockEntryView(row, col, numPtRows, numPtCols, blockEntry);
 
          Teuchos::SerialDenseMatrix<GO,Scalar> blk(blockSize,blockSize);
          blk.putScalar(row+col+1);

          Teuchos::ArrayRCP<const Scalar> blk_values(blk.values(), 0, blockSize*blockSize, false);
          TEST_COMPARE_FLOATING_ARRAYS( blockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
        }
      }
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, SetAndGetBlockEntry2, LO, GO, Scalar, Node )
  {
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
      }
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          LO numPtRows, numPtCols;
          Teuchos::ArrayRCP<const Scalar> blockEntry;
          vbr->getGlobalBlockEntryView(row, col, numPtRows, numPtCols, blockEntry);
 
          Teuchos::SerialDenseMatrix<GO,Scalar> blk(blockSize,blockSize);
          blk.putScalar(row+col+1);

          Teuchos::ArrayRCP<const Scalar> blk_values(blk.values(), 0, blockSize*blockSize, false);
          TEST_COMPARE_FLOATING_ARRAYS( blockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
        }
      }
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, SetAndGetBlockEntry3, LO, GO, Scalar, Node )
  {
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
          //setLocalBlockEntry should throw since fillComplete hasn't been called yet:
          TEST_THROW(vbr->setLocalBlockEntry(i, j, blockSize, blockSize, blockSize, blkEntry()), std::runtime_error);
        }
      }

      vbr->fillComplete();

      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          LO numPtRows, numPtCols;
          Teuchos::ArrayRCP<const Scalar> blockEntry;
          vbr->getGlobalBlockEntryView(row, col, numPtRows, numPtCols, blockEntry);
 
          Teuchos::ArrayRCP<Scalar> nonconstblockEntry;
          vbr->getGlobalBlockEntryViewNonConst(row, col, numPtRows, numPtCols, nonconstblockEntry);
 
          Teuchos::SerialDenseMatrix<GO,Scalar> blk(blockSize,blockSize);
          blk.putScalar(row+col+1);

          Teuchos::ArrayRCP<const Scalar> blk_values(blk.values(), 0, blockSize*blockSize, false);
          TEST_COMPARE_FLOATING_ARRAYS( blockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
          TEST_COMPARE_FLOATING_ARRAYS( nonconstblockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
        }
      }
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, putScalarAndSumInto1, LO, GO, Scalar, Node )
  {
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
      }

      vbr->fillComplete();
      vbr->putScalar(10.0);

      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          vbr->sumIntoGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
      }

      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          vbr->sumIntoLocalBlockEntry(i, j, blockSize, blockSize, blockSize, blkEntry());
        }
      }

      for(int i=0; i<blk_rows.size(); ++i) {
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          LO numPtRows, numPtCols;
          Teuchos::ArrayRCP<const Scalar> blockEntry;
          vbr->getGlobalBlockEntryView(row, col, numPtRows, numPtCols, blockEntry);
 
          Teuchos::ArrayRCP<Scalar> nonconstblockEntry;
          vbr->getGlobalBlockEntryViewNonConst(row, col, numPtRows, numPtCols, nonconstblockEntry);
 
          Teuchos::SerialDenseMatrix<GO,Scalar> blk(blockSize,blockSize);
          blk.putScalar(2*(row+col+1)+10.0);

          Teuchos::ArrayRCP<const Scalar> blk_values(blk.values(), 0, blockSize*blockSize, false);
          TEST_COMPARE_FLOATING_ARRAYS( blockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
          TEST_COMPARE_FLOATING_ARRAYS( nonconstblockEntry, blk_values, 2*Teuchos::ScalarTraits<Scalar>::eps());
        }
      }
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, applySimple, LO, GO, Scalar, Node )
  {
    //This test builds a block-diagonal matrix and tests the apply method.
    //Since the matrix is block-diagonal, the apply does not require any
    //communication. (No import of the X vector is required.)
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<BMV> bmv1 = rcp(new BMV(rowmap, 1));
      RCP<BMV> bmv2 = rcp(new BMV(rowmap, 1));
      RCP<BMV> bmv3 = rcp(new BMV(rowmap, 1));
      bmv1->putScalar(1.0);
      ArrayRCP<Scalar> v3 = bmv3->get1dViewNonConst();
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      LO row_offset = 0;
      for(int i=0; i<blk_rows.size(); ++i) {
        Scalar val = 0;
        GO row = blk_rows[i];
        for(int j=0; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          val += (row+col+1)*blockSize;
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
        for(int k=0; k<blockSize; ++k) v3[row_offset++] = val;
      }

      vbr->fillComplete();
      vbr->apply(*bmv1, *bmv2);
      ArrayRCP<Scalar> v2 = bmv2->get1dViewNonConst();
      TEST_COMPARE_FLOATING_ARRAYS( v2, v3, 2*Teuchos::ScalarTraits<Scalar>::eps());
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, solveUpperNonUnit, LO, GO, Scalar, Node )
  {
    //This test builds a block-diagonal, upper-triangular matrix and tests the
    //solve method.
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 3;
    const LO blockSize = 3;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        Scalar val = 0;
        GO row = blk_rows[i];
        for(int j=i; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          if (row==col) zero_lower_triangle(blockSize, blkEntry);
          val += (row+col+1)*blockSize;
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
      }

      vbr->fillComplete();

      RCP<BMV> x = rcp(new BMV(rowmap, 1));
      x->putScalar(1.0);
      RCP<BMV> y = rcp(new BMV(rowmap, 1));
      RCP<BMV> x2 = rcp(new BMV(rowmap, 1));

      vbr->apply(*x, *y);
      vbr->applyInverse(*y, *x2, Teuchos::NO_TRANS);
      ArrayRCP<const Scalar> v_x = x->get1dView();
      ArrayRCP<const Scalar> v_x2 = x2->get1dView();
      TEST_COMPARE_FLOATING_ARRAYS( v_x, v_x2, 2*Teuchos::ScalarTraits<Scalar>::eps());
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, solveTransposeUpperNonUnit, LO, GO, Scalar, Node )
  {
    //This test builds a block-diagonal, upper-triangular matrix and tests the
    //transpose-solve method.
    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 3;
    const LO blockSize = 3;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );
    // create the matrix
    {
      RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
      Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
      for(int i=0; i<blk_rows.size(); ++i) {
        Scalar val = 0;
        GO row = blk_rows[i];
        for(int j=i; j<blk_rows.size(); ++j) {
          GO col = blk_rows[j];
          Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
          if (row==col) zero_lower_triangle(blockSize, blkEntry);
          val += (row+col+1)*blockSize;
          vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        }
      }

      vbr->fillComplete();

      RCP<BMV> x = rcp(new BMV(rowmap, 1));
      x->putScalar(1.0);
      RCP<BMV> y = rcp(new BMV(rowmap, 1));
      RCP<BMV> x2 = rcp(new BMV(rowmap, 1));

      vbr->apply(*x, *y, Teuchos::TRANS);
      vbr->applyInverse(*y, *x2, Teuchos::TRANS);
      ArrayRCP<const Scalar> v_x = x->get1dView();
      ArrayRCP<const Scalar> v_x2 = x2->get1dView();
      TEST_COMPARE_FLOATING_ARRAYS( v_x, v_x2, 2*Teuchos::ScalarTraits<Scalar>::eps());
    }
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, ColMap1, LO, GO, Scalar, Node )
  {
    //This test fills a (block-tri-diagonal) matrix such that in parallel
    //the column-map should have an overlapping set of entries (i.e.,
    //different than the row-map), and verify that the column-map is correct.

    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    GO maxGlobalBlock = numLocalBlocks*comm->getSize();
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );

    //now set up the list of block-column-ids that we expect the
    //column-map to contain after fillComplete:
    size_t numLocalColBlocks = numLocalBlocks;
    if (comm->getRank() != 0) ++numLocalColBlocks;
    if (comm->getRank() != comm->getSize()-1) ++numLocalColBlocks;
    Array<GO> blockColIDs(numLocalColBlocks);
    typedef typename Array<GO>::size_type Tsize_t;
    Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
    GO first_row = blk_rows[0];
    Tsize_t offset = 0;
    if (comm->getRank() != 0) {
      blockColIDs[offset++] = first_row - 1;
    }
    GO last_row = 0;
    for(LO i=0; i<blk_rows.size(); ++i) {
      blockColIDs[offset++] = blk_rows[i];
      last_row = blk_rows[i];
    }
    if (offset < blockColIDs.size()) blockColIDs[offset++] = last_row + 1;

    // create the matrix
    RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
    for(int i=0; i<blk_rows.size(); ++i) {
      GO row = blk_rows[i];
      {
        GO col = row;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
      }
      if (row > indexBase) {
        GO col = row - 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
      }
      if (row < maxGlobalBlock-1) {
        GO col = row + 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
      }
    }

    vbr->fillComplete();
    RCP<const BlockMap<LO,GO,Node> > colmap = vbr->getBlockColMap();
    ArrayView<const GO> blk_cols = colmap->getNodeBlockIDs();
    TEST_EQUALITY(blk_cols.size(), blockColIDs.size());
    TEST_COMPARE_ARRAYS(blk_cols, blockColIDs() );
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, applyParallel, LO, GO, Scalar, Node )
  {
    //This test fills a (block-tri-diagonal) matrix such that in parallel the
    //column-map should have an overlapping set of entries (i.e., different than
    //the row-map), and verify that apply works correctly. If the column-map
    //has an overlapping set of entries, then apply must do an import of the x
    //vector in order to get a correct result.

    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    GO maxGlobalBlock = numLocalBlocks*comm->getSize();
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );

    //now set up the list of block-column-ids that we expect the
    //column-map to contain after fillComplete:
    size_t numLocalColBlocks = numLocalBlocks;
    if (comm->getRank() != 0) ++numLocalColBlocks;
    if (comm->getRank() != comm->getSize()-1) ++numLocalColBlocks;
    Array<GO> blockColIDs(numLocalColBlocks);
    typedef typename Array<GO>::size_type Tsize_t;
    Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
    GO first_row = blk_rows[0];
    Tsize_t offset = 0;
    if (comm->getRank() != 0) {
      blockColIDs[offset++] = first_row - 1;
    }
    GO last_row = 0;
    LO row_offset = 0;
    for(LO i=0; i<blk_rows.size(); ++i) {
      blockColIDs[offset++] = blk_rows[i];
      last_row = blk_rows[i];
    }
    if (offset < blockColIDs.size()) blockColIDs[offset++] = last_row + 1;

    RCP<BMV> bmv1 = rcp(new BMV(rowmap, 1));
    RCP<BMV> bmv2 = rcp(new BMV(rowmap, 1));
    RCP<BMV> bmv3 = rcp(new BMV(rowmap, 1));
    bmv1->putScalar(1.0);
    ArrayRCP<Scalar> v3 = bmv3->get1dViewNonConst();

    // create the matrix
    RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
    for(int i=0; i<blk_rows.size(); ++i) {
      Scalar val = 0;
      GO row = blk_rows[i];
      {
        GO col = row;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row > indexBase) {
        GO col = row - 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row < maxGlobalBlock-1) {
        GO col = row + 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      for(int k=0; k<blockSize; ++k) v3[row_offset++] = val;
    }

    vbr->fillComplete();

    vbr->apply(*bmv1, *bmv2);

    ArrayRCP<Scalar> v2 = bmv2->get1dViewNonConst();
    TEST_COMPARE_FLOATING_ARRAYS( v2, v3, 2*Teuchos::ScalarTraits<Scalar>::eps());
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, applyTransParallel, LO, GO, Scalar, Node )
  {
    //This test fills a (block-tri-diagonal) matrix such that in parallel the
    //column-map should have an overlapping set of entries (i.e., different than
    //the row-map), and verify that apply transpose works correctly. If the column-map
    //has an overlapping set of entries, then apply must do an export of the y
    //vector in order to get a correct result.

    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    GO numGlobalBlocks = numLocalBlocks*comm->getSize();
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );

    //now set up the list of block-column-ids that we expect the
    //column-map to contain after fillComplete:
    size_t numLocalColBlocks = numLocalBlocks;
    if (comm->getRank() != 0) ++numLocalColBlocks;
    if (comm->getRank() != comm->getSize()-1) ++numLocalColBlocks;
    Array<GO> blockColIDs(numLocalColBlocks);
    typedef typename Array<GO>::size_type Tsize_t;
    Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
    GO first_row = blk_rows[0];
    Tsize_t offset = 0;
    if (comm->getRank() != 0) {
      blockColIDs[offset++] = first_row - 1;
    }
    GO last_row = 0;
    LO row_offset = 0;
    for(LO i=0; i<blk_rows.size(); ++i) {
      blockColIDs[offset++] = blk_rows[i];
      last_row = blk_rows[i];
    }
    if (offset < blockColIDs.size()) blockColIDs[offset++] = last_row + 1;

    RCP<BMV> bmv1 = rcp(new BMV(rowmap, 1));
    RCP<BMV> bmv2 = rcp(new BMV(rowmap, 1));
    RCP<BMV> bmv3 = rcp(new BMV(rowmap, 1));
    bmv1->putScalar(1.0);
    ArrayRCP<Scalar> v3 = bmv3->get1dViewNonConst();

    // create the matrix
    RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
    for(int i=0; i<blk_rows.size(); ++i) {
      Scalar val = 0;
      GO row = blk_rows[i];
      {
        GO col = row;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row > indexBase) {
        GO col = row - 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row < numGlobalBlocks-1) {
        GO col = row + 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      for(int k=0; k<blockSize; ++k) v3[row_offset++] = val;
    }

    vbr->fillComplete();

    vbr->apply(*bmv1, *bmv2, Teuchos::TRANS);

    ArrayRCP<Scalar> v2 = bmv2->get1dViewNonConst();
    TEST_COMPARE_FLOATING_ARRAYS( v2, v3, 2*Teuchos::ScalarTraits<Scalar>::eps());
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, applyParallelMV, LO, GO, Scalar, Node )
  {
    //This test fills a (block-tri-diagonal) matrix such that in parallel the
    //column-map should have an overlapping set of entries (i.e., different than
    //the row-map), and verify that apply works correctly. If the column-map
    //has an overlapping set of entries, then apply must do an import of the x
    //vector in order to get a correct result.
    //This test uses MultiVectors that have more than 1 column.

    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    GO maxGlobalBlock = numLocalBlocks*comm->getSize();
    const LO blockSize = 2;
    const size_t maxEntriesPerRow = 3;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );

    //now set up the list of block-column-ids that we expect the
    //column-map to contain after fillComplete:
    size_t numLocalColBlocks = numLocalBlocks;
    if (comm->getRank() != 0) ++numLocalColBlocks;
    if (comm->getRank() != comm->getSize()-1) ++numLocalColBlocks;
    Array<GO> blockColIDs(numLocalColBlocks);
    typedef typename Array<GO>::size_type Tsize_t;
    Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
    GO first_row = blk_rows[0];
    Tsize_t offset = 0;
    if (comm->getRank() != 0) {
      blockColIDs[offset++] = first_row - 1;
    }
    GO last_row = 0;
    LO row_offset = 0;
    for(LO i=0; i<blk_rows.size(); ++i) {
      blockColIDs[offset++] = blk_rows[i];
      last_row = blk_rows[i];
    }
    if (offset < blockColIDs.size()) blockColIDs[offset++] = last_row + 1;

    size_t numVecs = 3;
    size_t numPointRows = rowmap->getPointMap()->getNodeNumElements();
    RCP<BMV> bmv1 = rcp(new BMV(rowmap, numVecs));
    RCP<BMV> bmv2 = rcp(new BMV(rowmap, numVecs));
    RCP<BMV> bmv3 = rcp(new BMV(rowmap, numVecs));
    bmv1->putScalar(1.0);
    ArrayRCP<Scalar> v3 = bmv3->get1dViewNonConst();

    // create the matrix
    RCP<MAT> vbr = rcp( new MAT(rowmap,maxEntriesPerRow,DynamicProfile) );
    for(int i=0; i<blk_rows.size(); ++i) {
      Scalar val = 0;
      GO row = blk_rows[i];
      {
        GO col = row;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row > indexBase) {
        GO col = row - 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row < maxGlobalBlock-1) {
        GO col = row + 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      for(int k=0; k<blockSize; ++k) {
        for(size_t v=0; v<numVecs; ++v) {
          v3[v*numPointRows+row_offset] = val;
        }
        ++row_offset;
      }
    }

    vbr->fillComplete();

    vbr->apply(*bmv1, *bmv2);

    ArrayRCP<Scalar> v2 = bmv2->get1dViewNonConst();
    TEST_COMPARE_FLOATING_ARRAYS( v2, v3, 2*Teuchos::ScalarTraits<Scalar>::eps());
  }

  ////
  TEUCHOS_UNIT_TEST_TEMPLATE_4_DECL( VbrMatrix, CtorBCrsGraph, LO, GO, Scalar, Node )
  {
    //This test creates a (block-tri-diagonal) matrix using a pre-filled
    //BlockCrsGraph such that in parallel the column-map should have an
    //overlapping set of entries (i.e., different than the row-map),
    //and verify that apply works correctly. If the column-map
    //has an overlapping set of entries, then apply must do an import of the x
    //vector in order to get a correct result.

    RCP<Node> node = getNode<Node>();
    typedef ScalarTraits<Scalar> ST;
    typedef BlockCrsGraph<LO,GO,Node> BGRAPH;
    typedef BlockMultiVector<Scalar,LO,GO,Node> BMV;
    typedef VbrMatrix<Scalar,LO,GO,Node> MAT;
    typedef typename ST::magnitudeType Mag;
    typedef ScalarTraits<Mag> MT;
    const global_size_t INVALID = OrdinalTraits<global_size_t>::invalid();
    // get a comm
    RCP<const Comm<int> > comm = getDefaultComm();
    GO indexBase = 0;
    // create a Map
    const size_t numLocalBlocks = 2;
    GO maxGlobalBlock = numLocalBlocks*comm->getSize();
    const LO blockSize = 2;
    RCP<BlockMap<LO,GO,Node> > rowmap = rcp( new BlockMap<LO,GO,Node>(INVALID,numLocalBlocks,blockSize,indexBase,comm,node) );

    //now set up the list of block-column-ids that we expect the
    //column-map to contain after fillComplete:
    size_t numLocalColBlocks = numLocalBlocks;
    if (comm->getRank() != 0) ++numLocalColBlocks;
    if (comm->getRank() != comm->getSize()-1) ++numLocalColBlocks;
    Array<GO> blockColIDs(numLocalColBlocks);
    typedef typename Array<GO>::size_type Tsize_t;
    Teuchos::ArrayView<const GO> blk_rows = rowmap->getNodeBlockIDs();
    GO first_row = blk_rows[0];
    Tsize_t offset = 0;
    if (comm->getRank() != 0) {
      blockColIDs[offset++] = first_row - 1;
    }
    GO last_row = 0;
    for(LO i=0; i<blk_rows.size(); ++i) {
      blockColIDs[offset++] = blk_rows[i];
      last_row = blk_rows[i];
    }
    if (offset < blockColIDs.size()) blockColIDs[offset++] = last_row + 1;

    const size_t maxEntriesPerRow = 3;
    // create and fill the graph
    RCP<BGRAPH> bgrph = rcp( new BGRAPH(rowmap,maxEntriesPerRow,DynamicProfile) );
    for(int i=0; i<blk_rows.size(); ++i) {
      GO row = blk_rows[i];
      {
        GO col = row;
        bgrph->insertGlobalIndices(row, Teuchos::arrayView(&col, 1));
      }
      if (row > indexBase) {
        GO col = row - 1;
        bgrph->insertGlobalIndices(row, Teuchos::arrayView(&col, 1));
      }
      if (row < maxGlobalBlock-1) {
        GO col = row + 1;
        bgrph->insertGlobalIndices(row, Teuchos::arrayView(&col, 1));
      }
    }

    bgrph->fillComplete();

    RCP<BMV> bmv3 = rcp(new BMV(rowmap, 1));
    ArrayRCP<Scalar> v3 = bmv3->get1dViewNonConst();

    // create and fill the matrix
    RCP<MAT> vbr = rcp( new MAT(bgrph) );
    LO row_offset = 0;
    for(int i=0; i<blk_rows.size(); ++i) {
      Scalar val = 0;
      GO row = blk_rows[i];
      {
        GO col = row;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row > indexBase) {
        GO col = row - 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      if (row < maxGlobalBlock-1) {
        GO col = row + 1;
        Teuchos::Array<Scalar> blkEntry(blockSize * blockSize, row+col+1);
        vbr->setGlobalBlockEntry(row, col, blockSize, blockSize, blockSize, blkEntry());
        val += (row+col+1)*blockSize;
      }
      for(int k=0; k<blockSize; ++k) v3[row_offset++] = val;
    }

    vbr->fillComplete();

    RCP<BMV> bmv1 = rcp(new BMV(rowmap, 1));
    RCP<BMV> bmv2 = rcp(new BMV(rowmap, 1));
    bmv1->putScalar(1.0);

    vbr->apply(*bmv1, *bmv2);

    ArrayRCP<Scalar> v2 = bmv2->get1dViewNonConst();
    TEST_COMPARE_FLOATING_ARRAYS( v2, v3, 2*Teuchos::ScalarTraits<Scalar>::eps());
  }

// 
// INSTANTIATIONS
//


// Uncomment this for really fast development cycles but make sure to comment
// it back again before checking in so that we can test all the types.
#define FAST_DEVELOPMENT_UNIT_TEST_BUILD

typedef std::complex<float>  ComplexFloat;
typedef std::complex<double> ComplexDouble;

#define UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, Basic, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, SetAndGetBlockEntry1, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, SetAndGetBlockEntry2, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, SetAndGetBlockEntry3, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, putScalarAndSumInto1, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, applySimple, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, solveUpperNonUnit, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, solveTransposeUpperNonUnit, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, ColMap1, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, applyParallel, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, applyTransParallel, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, applyParallelMV, LO, GO, SCALAR, NODE ) \
      TEUCHOS_UNIT_TEST_TEMPLATE_4_INSTANT( VbrMatrix, CtorBCrsGraph, LO, GO, SCALAR, NODE )


#define UNIT_TEST_SERIALNODE(LO, GO, SCALAR) \
      UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, SCALAR, SerialNode )

#ifdef HAVE_KOKKOS_TBB
#define UNIT_TEST_TBBNODE(LO, GO, SCALAR) \
      UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, SCALAR, TBBNode )
#else
#define UNIT_TEST_TBBNODE(LO, GO, SCALAR)
#endif

#ifdef HAVE_KOKKOS_THREADPOOL
#define UNIT_TEST_TPINODE(LO, GO, SCALAR) \
      UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, SCALAR, TPINode )
#else
#define UNIT_TEST_TPINODE(LO, GO, SCALAR)
#endif

// don't test Kokkos node for MPI builds, because we probably don't have multiple GPUs per node
#if defined(HAVE_KOKKOS_THRUST) && !defined(HAVE_TPETRA_MPI)
// float
#if defined(HAVE_KOKKOS_CUDA_FLOAT)
#  define UNIT_TEST_THRUSTGPUNODE_FLOAT(LO, GO) \
          UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, float, ThrustGPUNode )
#else
#  define UNIT_TEST_THRUSTGPUNODE_FLOAT(LO, GO)
#endif
// double
#if defined(HAVE_KOKKOS_CUDA_DOUBLE)
#  define UNIT_TEST_THRUSTGPUNODE_DOUBLE(LO, GO) \
          UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, double, ThrustGPUNode )
#else
#  define UNIT_TEST_THRUSTGPUNODE_DOUBLE(LO, GO)
#endif
// complex<float>
#if defined(HAVE_KOKKOS_CUDA_COMPLEX_FLOAT)
#  define UNIT_TEST_THRUSTGPUNODE_COMPLEX_FLOAT(LO, GO) \
          UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, ComplexFloat, ThrustGPUNode )
#else
#  define UNIT_TEST_THRUSTGPUNODE_COMPLEX_FLOAT(LO, GO)
#endif
// complex<double>
#if defined(HAVE_KOKKOS_CUDA_COMPLEX_DOUBLE)
#  define UNIT_TEST_THRUSTGPUNODE_COMPLEX_DOUBLE(LO, GO) \
          UNIT_TEST_GROUP_ORDINAL_SCALAR_NODE( LO, GO, ComplexDouble, ThrustGPUNode )
#else
#  define UNIT_TEST_THRUSTGPUNODE_COMPLEX_DOUBLE(LO, GO)
#endif
#else
// none
# define UNIT_TEST_THRUSTGPUNODE_FLOAT(LO, GO)
# define UNIT_TEST_THRUSTGPUNODE_DOUBLE(LO, GO)
# define UNIT_TEST_THRUSTGPUNODE_COMPLEX_FLOAT(LO, GO)
# define UNIT_TEST_THRUSTGPUNODE_COMPLEX_DOUBLE(LO, GO)
#endif

#define UNIT_TEST_ALLCPUNODES(LO, GO, SCALAR) \
    UNIT_TEST_SERIALNODE(LO, GO, SCALAR) \
    UNIT_TEST_TBBNODE(LO, GO, SCALAR) \
    UNIT_TEST_TPINODE(LO, GO, SCALAR)

#define UNIT_TEST_FLOAT(LO, GO) \
    UNIT_TEST_ALLCPUNODES(LO, GO, float) \
    UNIT_TEST_THRUSTGPUNODE_FLOAT(LO, GO)

#define UNIT_TEST_DOUBLE(LO, GO) \
    UNIT_TEST_ALLCPUNODES(LO, GO, double) \
    UNIT_TEST_THRUSTGPUNODE_DOUBLE(LO, GO)

#define UNIT_TEST_COMPLEX_FLOAT(LO, GO) \
    UNIT_TEST_ALLCPUNODES(LO, GO, ComplexFloat) \
    UNIT_TEST_THRUSTGPUNODE_COMPLEX_FLOAT(LO, GO)

#define UNIT_TEST_COMPLEX_DOUBLE(LO, GO) \
    UNIT_TEST_ALLCPUNODES(LO, GO, ComplexDouble) \
    UNIT_TEST_THRUSTGPUNODE_COMPLEX_DOUBLE(LO, GO)

#if defined(HAVE_TPETRA_INST_DOUBLE)
  UNIT_TEST_DOUBLE(int, int)
#endif

#if !defined(FAST_DEVELOPMENT_UNIT_TEST_BUILD)
# if defined(HAVE_TPETRA_INST_FLOAT)
    UNIT_TEST_FLOAT(int, int)
# endif 
# if defined(HAVE_TPETRA_INST_COMPLEX_FLOAT)
    UNIT_TEST_COMPLEX_FLOAT(int, int)
# endif 
# if defined(HAVE_TPETRA_INST_COMPLEX_DOUBLE)
    UNIT_TEST_COMPLEX_DOUBLE(int, int)
# endif 
#endif // FAST_DEVELOPMENT_UNIT_TEST_BUILD

}

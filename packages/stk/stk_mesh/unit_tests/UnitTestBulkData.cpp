/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/


#include <sstream>
#include <stdexcept>

#include <stk_util/unit_test_support/stk_utest_macros.hpp>

#include <stk_util/parallel/Parallel.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/EntityComm.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/fem/TopologicalMetaData.hpp>

#include <unit_tests/UnitTestBulkData.hpp>
#include <unit_tests/UnitTestRingMeshFixture.hpp>

STKUNIT_UNIT_TEST(UnitTestingOfBulkData, testUnit)
{
  MPI_Barrier( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testBulkData( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testChangeOwner_nodes( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testChangeOwner_loop( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testChangeOwner_box( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testCreateMore( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testChangeParts( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testChangeParts_loop( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testDestroy_nodes( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testDestroy_loop( MPI_COMM_WORLD );
  stk::mesh::UnitTestBulkData::testModifyPropagation( MPI_COMM_WORLD );
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------



namespace stk {
namespace mesh {

bool UnitTestBulkData::modification_end( BulkData & mesh , bool aura )
{ return mesh.internal_modification_end( aura ); }

// Unit test the Part functionality in isolation:

void UnitTestBulkData::testBulkData( ParallelMachine pm )
{
  static const char method[] = "stk::mesh::UnitTestBulkData" ;

  std::cout << std::endl ;

  std::vector<std::string> entity_names(10);
  for ( size_t i = 0 ; i < 10 ; ++i ) {
    std::ostringstream name ;
    name << "EntityRank" << i ;
    entity_names[i] = name.str();
  }

  MetaData meta( entity_names );

  meta.commit();

  BulkData bulk( meta , pm , 100 );

  for ( size_t i = 0 ; i < 4 ; ++i ) {
    STKUNIT_ASSERT( bulk.modification_begin() );
    STKUNIT_ASSERT_EQUAL( i , bulk.synchronized_count() );
    STKUNIT_ASSERT( bulk.modification_end() );
  }

  std::vector<Part*> no_parts ;

  Entity * e[10] ;

  const unsigned id = bulk.parallel_rank() + 1 ;

  STKUNIT_ASSERT( bulk.modification_begin() );
  for ( size_t i = 0 ; i < 10 ; ++i ) {
    e[i] = & bulk.declare_entity(  i , id , no_parts );
  }
  STKUNIT_ASSERT( bulk.modification_end() );

  for ( size_t i = 0 ; i < 10 ; ++i ) {
    STKUNIT_ASSERT( e[i] == bulk.get_entity( i , id ) );
  }

  bool ok = false ;
  STKUNIT_ASSERT( bulk.modification_begin() );
  try {
    bulk.declare_entity( 11 , id , no_parts );
  }
  catch( const std::exception & x ) {
    std::cout << method << " correctly caught: " << x.what() << std::endl ;
    ok = true ;
  }
  STKUNIT_ASSERT( ok );
  STKUNIT_ASSERT( bulk.modification_end() );

  // Catch not-ok-to-modify
  ok = false ;
  try {
    bulk.declare_entity( 0 , id + 1 , no_parts );
  }
  catch( const std::exception & x ) {
    ok = true ;
  }
  STKUNIT_ASSERT( ok );
}

//----------------------------------------------------------------------
// Testing for mesh entities without relations

void UnitTestBulkData::testChangeOwner_nodes( ParallelMachine pm )
{
  enum { nPerProc = 10 };
  const unsigned p_rank = parallel_machine_rank( pm );
  const unsigned p_size = parallel_machine_size( pm );
  const unsigned id_total = nPerProc * p_size ;
  const unsigned id_begin = nPerProc * p_rank ;
  const unsigned id_end   = nPerProc * ( p_rank + 1 );

  const int spatial_dimension = 3;
  MetaData meta( TopologicalMetaData::entity_rank_names(spatial_dimension) );
  BulkData bulk( meta , pm , 100 );

  const PartVector no_parts ;

  meta.commit();
  bulk.modification_begin();

  // Ids for all entities (all entities have type 0):

  std::vector<EntityId> ids( id_total );

  for ( unsigned i = 0 ; i < id_total ; ++i ) {
    ids[i] = i + 1;
  }

  // Declare just those entities in my range of ids:

  for ( unsigned i = id_begin ; i < id_end ; ++i ) {
    bulk.declare_entity( 0 , ids[i] , no_parts );
  }

  STKUNIT_ASSERT( bulk.modification_end() );

  // Verify that I only have entities in my range:

  for ( unsigned i = 0 ; i < id_total ; ++i ) {
    Entity * e = bulk.get_entity( 0 , ids[ i ] );
    if ( id_begin <= i && i < id_end ) {
      STKUNIT_ASSERT( NULL != e );
    }
    else {
      STKUNIT_ASSERT( NULL == e );
    }
  }

  // Test change owner no-op first:

  std::vector<EntityProc> change ;

  STKUNIT_ASSERT( bulk.modification_begin() );
  bulk.change_entity_owner( change );
  STKUNIT_ASSERT( bulk.modification_end() );

  for ( unsigned i = 0 ; i < id_total ; ++i ) {
    Entity * e = bulk.get_entity( 0 , ids[ i ] );
    if ( id_begin <= i && i < id_end ) {
      STKUNIT_ASSERT( NULL != e );
    }
    else {
      STKUNIT_ASSERT( NULL == e );
    }
  }

  // Can only test changing owner in parallel.

  if ( 1 < p_size ) {
    // Give my last two ids to the next process
    // Get the previous process' last two ids

    const unsigned p_give = ( p_rank + 1 ) % p_size ;
    const unsigned id_give = id_end - 2 ;
    const unsigned id_get  = ( id_begin + id_total - 2 ) % id_total ;

    STKUNIT_ASSERT( NULL != bulk.get_entity( 0 , ids[id_give] ) );
    STKUNIT_ASSERT( NULL != bulk.get_entity( 0 , ids[id_give+1] ) );
    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_get] ) );
    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_get+1] ) );

    change.resize(2);
    change[0].first = bulk.get_entity( 0 , ids[id_give] );
    change[0].second = p_give ;
    change[1].first = bulk.get_entity( 0 , ids[id_give+1] );
    change[1].second = p_give ;

    STKUNIT_ASSERT( bulk.modification_begin() );
    bulk.change_entity_owner( change );
    STKUNIT_ASSERT( bulk.modification_end() );

    STKUNIT_ASSERT( NULL != bulk.get_entity( 0 , ids[id_get] ) );
    STKUNIT_ASSERT( NULL != bulk.get_entity( 0 , ids[id_get+1] ) );

    // Entities given away are destroyed until the next modification cycle
    {
      Entity * const e0 = bulk.get_entity( 0 , ids[id_give] );
      Entity * const e1 = bulk.get_entity( 0 , ids[id_give+1] );
      STKUNIT_ASSERT( NULL != e0 && e0->bucket().capacity() == 0 );
      STKUNIT_ASSERT( NULL != e1 && e1->bucket().capacity() == 0 );
    }

    STKUNIT_ASSERT( bulk.modification_begin() );
    STKUNIT_ASSERT( bulk.modification_end() );

    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_give] ) );
    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_give+1] ) );
  }

  std::cout << std::endl
            << "P" << p_rank
            << ": UnitTestBulkData::testChangeOwner_nodes( NP = "
            << p_size << " ) SUCCESSFULL " << std::endl ;
}

//----------------------------------------------------------------------
// Testing for creating existing mesh entities without relations

void UnitTestBulkData::testCreateMore( ParallelMachine pm )
{
  enum { nPerProc = 10 };

  const unsigned p_size = parallel_machine_size( pm );
  const unsigned p_rank = parallel_machine_rank( pm );

  if ( 1 < p_size ) {

    std::cout << std::endl
              << "P" << p_rank
              << ": UnitTestBulkData::testCreateMore( NP = "
              << p_size << " )"
              << std::endl ;
    std::cout.flush();

    const unsigned id_total = nPerProc * p_size ;
    const unsigned id_begin = nPerProc * p_rank ;
    const unsigned id_end   = nPerProc * ( p_rank + 1 );

    const int spatial_dimension = 3;
    MetaData meta( TopologicalMetaData::entity_rank_names(spatial_dimension) );

    const PartVector no_parts ;

    meta.commit();

    BulkData bulk( meta , pm , 100 );

    bulk.modification_begin();

    // Ids for all entities (all entities have type 0):

    std::vector<EntityId> ids( id_total );

    for ( unsigned i = 0 ; i < id_total ; ++i ) { ids[i] = i + 1; }

    // Declare just those entities in my range of ids:

    for ( unsigned i = id_begin ; i < id_end ; ++i ) {
      bulk.declare_entity( 0 , ids[i] , no_parts );
    }

    STKUNIT_ASSERT( bulk.modification_end() );

    // Only one process create entities with previous process' last two ids

    const unsigned id_get  = ( id_begin + id_total - 2 ) % id_total ;

    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_get] ) );
    STKUNIT_ASSERT( NULL == bulk.get_entity( 0 , ids[id_get+1] ) );

    STKUNIT_ASSERT( bulk.modification_begin() );

    if ( 1 == p_rank ) {
      // These declarations create entities that already exist,
      // which will be an error.  Must create an owned entity
      // to use them, thus they become shared.

      Entity & e0 = bulk.declare_entity( 0 , ids[ id_get ] , no_parts );
      Entity & e1 = bulk.declare_entity( 0 , ids[ id_get + 1 ] , no_parts );

      Entity & eU = bulk.declare_entity( 1 , 1 , no_parts );

      bulk.declare_relation( eU , e0 , 0 );
      bulk.declare_relation( eU , e1 , 1 );
    }

    bulk.modification_end();

    if ( 1 == p_rank ) {
      Entity * e0 = bulk.get_entity( 0 , ids[id_get] );
      Entity * e1 = bulk.get_entity( 0 , ids[id_get+1] );
      STKUNIT_ASSERT( NULL != e0 );
      STKUNIT_ASSERT( NULL != e1 );
      STKUNIT_ASSERT( 0 == e0->owner_rank() );
      STKUNIT_ASSERT( 0 == e1->owner_rank() );
    }

    // Now test tripping the error condition

    bulk.modification_begin();

    if ( 0 == p_rank ) {
      bulk.declare_entity( 0 , ids[ id_get ] , no_parts );
      bulk.declare_entity( 0 , ids[ id_get + 1 ] , no_parts );
    }

    STKUNIT_ASSERT_THROW( bulk.modification_end() , std::runtime_error );
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

void UnitTestBulkData::testChangeOwner_loop( ParallelMachine pm )
{
  enum { nPerProc = 10 };
  const unsigned p_rank = parallel_machine_rank( pm );
  const unsigned p_size = parallel_machine_size( pm );
  const unsigned nLocalNode = nPerProc + ( 1 < p_size ? 1 : 0 );
  const unsigned nLocalEdge = nPerProc ;

  std::vector<unsigned> local_count ;

  //------------------------------
  {
    UnitTestRingMeshFixture ring_mesh( pm , nPerProc , false /* no edge parts */ );
    ring_mesh.m_meta_data.commit();
    ring_mesh.generate_mesh( false /* no aura */ );

    const Selector select_used = ring_mesh.m_meta_data.locally_owned_part() |
                                 ring_mesh.m_meta_data.globally_shared_part() ;
    const Selector select_all = ring_mesh.m_meta_data.universal_part() ;

    count_entities( select_used , ring_mesh.m_bulk_data , local_count );
    STKUNIT_ASSERT_EQUAL( local_count[0] , nLocalNode );
    STKUNIT_ASSERT_EQUAL( local_count[1] , nLocalEdge );

    count_entities( select_all , ring_mesh.m_bulk_data , local_count );
    STKUNIT_ASSERT_EQUAL( local_count[0] , nLocalNode );
    STKUNIT_ASSERT_EQUAL( local_count[1] , nLocalEdge );

    // Shift loop by two nodes and edges.

    if ( 1 < p_size ) {
      ring_mesh.test_shift_loop( false /* no aura */ );

      count_entities( select_used , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nLocalNode );
      STKUNIT_ASSERT( local_count[1] == nLocalEdge );

      count_entities( select_all , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nLocalNode );
      STKUNIT_ASSERT( local_count[1] == nLocalEdge );
    }
  }

  //------------------------------
  // Test shift starting with ghosting but not regenerated ghosting.
  {
    UnitTestRingMeshFixture ring_mesh( pm , nPerProc , false /* no edge parts */ );
    ring_mesh.m_meta_data.commit();
    ring_mesh.generate_mesh( true /* with aura */ );

    const Selector select_owned( ring_mesh.m_meta_data.locally_owned_part() );
    const Selector select_used = ring_mesh.m_meta_data.locally_owned_part() |
                                 ring_mesh.m_meta_data.globally_shared_part() ;
    const Selector select_all(   ring_mesh.m_meta_data.universal_part() );

    count_entities( select_used , ring_mesh.m_bulk_data , local_count );
    STKUNIT_ASSERT_EQUAL( local_count[0] , nLocalNode );
    STKUNIT_ASSERT_EQUAL( local_count[1] , nLocalEdge );

    count_entities( select_all , ring_mesh.m_bulk_data , local_count );
    const unsigned n_extra = 1 < p_size ? 2 : 0 ;
    STKUNIT_ASSERT( local_count[0] == nLocalNode + n_extra );
    STKUNIT_ASSERT( local_count[1] == nLocalEdge + n_extra );

    if ( 1 < p_size ) {
      ring_mesh.test_shift_loop( false /* no aura */ );

      count_entities( select_owned , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nPerProc );
      STKUNIT_ASSERT( local_count[1] == nPerProc );

      count_entities( select_used , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nLocalNode );
      STKUNIT_ASSERT( local_count[1] == nLocalEdge );

      // All of my ghosts were disrupted and therefore deleted:
      count_entities( select_all , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT_EQUAL( nLocalEdge , local_count[1] );
      STKUNIT_ASSERT_EQUAL( nLocalNode , local_count[0] );
    }
  }
  //------------------------------
  // Test shift starting with ghosting and regenerating ghosting.
  {
    UnitTestRingMeshFixture ring_mesh( pm , nPerProc , false /* no edge parts */ );
    ring_mesh.m_meta_data.commit();
    ring_mesh.generate_mesh( true /* with aura */ );

    const Selector select_owned( ring_mesh.m_meta_data.locally_owned_part() );
    const Selector select_used = ring_mesh.m_meta_data.locally_owned_part() |
                                 ring_mesh.m_meta_data.globally_shared_part() ;
    const Selector select_all(   ring_mesh.m_meta_data.universal_part() );

    count_entities( select_used , ring_mesh.m_bulk_data , local_count );
    STKUNIT_ASSERT( local_count[0] == nLocalNode );
    STKUNIT_ASSERT( local_count[1] == nLocalEdge );

    count_entities( select_all , ring_mesh.m_bulk_data , local_count );
    const unsigned n_extra = 1 < p_size ? 2 : 0 ;
    STKUNIT_ASSERT( local_count[0] == nLocalNode + n_extra );
    STKUNIT_ASSERT( local_count[1] == nLocalEdge + n_extra );

    if ( 1 < p_size ) {
      ring_mesh.test_shift_loop( true /* with aura */ );

      count_entities( select_owned , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nPerProc );
      STKUNIT_ASSERT( local_count[1] == nPerProc );

      count_entities( select_used , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nLocalNode );
      STKUNIT_ASSERT( local_count[1] == nLocalEdge );

      // All of my ghosts were regenerated:
      count_entities( select_all , ring_mesh.m_bulk_data , local_count );
      STKUNIT_ASSERT( local_count[0] == nLocalNode + n_extra );
      STKUNIT_ASSERT( local_count[1] == nLocalEdge + n_extra );
    }
  }
  //------------------------------
  // Test bad owner change catching:
  if ( 1 < p_size ) {
    UnitTestRingMeshFixture ring_mesh( pm , nPerProc , false /* no edge parts */ );
    ring_mesh.m_meta_data.commit();
    ring_mesh.generate_mesh( true /* with aura */ );

    std::vector<EntityProc> change ;

    if ( 0 == p_rank ) {
      change.resize(4);
      // Error to change to bad owner:
      change[0].first = ring_mesh.m_bulk_data.get_entity( 0 , ring_mesh.m_node_ids[1] );
      change[0].second = p_size ;
      // Error to change a ghost:
      for ( std::vector<Entity*>::const_iterator
            ec =  ring_mesh.m_bulk_data.entity_comm().begin() ;
            ec != ring_mesh.m_bulk_data.entity_comm().end() ; ++ec ) {
        if ( in_receive_ghost( **ec ) ) {
          change[1].first = *ec ;
          break ;
        }
      }
      change[1].second = p_rank ;
      // Error to change to multiple owners:
      change[2].first = ring_mesh.m_bulk_data.get_entity( 0 , ring_mesh.m_node_ids[1] );
      change[2].second = ( p_rank + 1 ) % p_size ;
      change[3].first = change[2].first ;
      change[3].second = ( p_rank + 2 ) % p_size ;
    }

    STKUNIT_ASSERT( ring_mesh.m_bulk_data.modification_begin() );

    std::string error_msg ;
    bool exception_thrown = false ;
    try {
      ring_mesh.m_bulk_data.change_entity_owner( change );
    }
    catch( const std::exception & x ) {
      exception_thrown = true ;
      error_msg.assign( x.what() );
    }
    STKUNIT_ASSERT( exception_thrown );
    std::cout
      << std::endl
      << "  UnitTestBulkData::testChangeOwner_loop SUCCESSFULLY CAUGHT: "
      << error_msg << std::endl ;
    std::cout.flush();
  }
  //------------------------------
  // Test move one element with initial ghosting but not regenerated ghosting:
  // last processor give its shared node to P0
  if ( 1 < p_size ) {
    UnitTestRingMeshFixture ring_mesh( pm , nPerProc , false /* no edge parts */ );
    ring_mesh.m_meta_data.commit();
    ring_mesh.generate_mesh( true /* with aura */ );

    const Selector select_owned( ring_mesh.m_meta_data.locally_owned_part() );
    const Selector select_used = ring_mesh.m_meta_data.locally_owned_part() |
                                 ring_mesh.m_meta_data.globally_shared_part() ;
    const Selector select_all(   ring_mesh.m_meta_data.universal_part() );

    std::vector<EntityProc> change ;

    if ( p_rank + 1 == p_size ) {
      EntityProc entry ;
      entry.first = ring_mesh.m_bulk_data.get_entity( 0 , ring_mesh.m_node_ids[0] );
      entry.second = 0 ;
      STKUNIT_ASSERT_EQUAL( p_rank , entry.first->owner_rank() );
      change.push_back( entry );
    }

    STKUNIT_ASSERT( ring_mesh.m_bulk_data.modification_begin() );
    ring_mesh.m_bulk_data.change_entity_owner( change );
    STKUNIT_ASSERT( UnitTestBulkData::modification_end( ring_mesh.m_bulk_data , false ) );

    count_entities( select_owned , ring_mesh.m_bulk_data , local_count );
    const unsigned n_node = p_rank == 0          ? nPerProc + 1 : (
                            p_rank + 1 == p_size ? nPerProc - 1 :
                                                   nPerProc );

    STKUNIT_ASSERT_EQUAL( n_node , local_count[0] );
    STKUNIT_ASSERT_EQUAL( (unsigned) nPerProc , local_count[1] );

    count_entities( select_used , ring_mesh.m_bulk_data , local_count );
    STKUNIT_ASSERT_EQUAL( nLocalNode , local_count[0] );
    STKUNIT_ASSERT_EQUAL( nLocalEdge , local_count[1] );

    // Moving the node disrupted ghosting on first and last process
    count_entities( select_all , ring_mesh.m_bulk_data , local_count );
    const unsigned n_extra = p_rank + 1 == p_size || p_rank == 0 ? 1 : 2 ;
    STKUNIT_ASSERT_EQUAL( nLocalNode + n_extra , local_count[0] );
    STKUNIT_ASSERT_EQUAL( nLocalEdge + n_extra , local_count[1] );
  }

  //------------------------------

  std::cout << std::endl
            << "P" << p_rank
            << ": UnitTestBulkData::testChangeOwner_loop( NP = "
            << p_size << " ) SUCCESSFULL" << std::endl ;
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// Testing for collection of boxes

namespace {

void donate_one_element( BulkData & mesh , bool aura )
{
  const unsigned p_rank = mesh.parallel_rank();

  Selector select_owned( mesh.mesh_meta_data().locally_owned_part() );

  std::vector<unsigned> before_count ;
  std::vector<unsigned> after_count ;

  count_entities( select_owned , mesh , before_count );

  // Change owner of an element on a process boundary
  // from P0 to P1, and then recount to confirm ownership change

  std::vector<EntityProc> change ;

  // A shared node:
  Entity * node = NULL ;
  Entity * elem = NULL ;

  for ( std::vector<Entity*>::const_iterator
        i =  mesh.entity_comm().begin() ;
        i != mesh.entity_comm().end() ; ++i ) {
    if ( in_shared( **i ) && (**i).entity_rank() == BaseEntityRank ) {
      node = *i ;
      break ;
    }
  }

  STKUNIT_ASSERT( node != NULL );

  for ( PairIterRelation rel = node->relations( 3 );
        ! rel.empty() && elem == NULL ; ++rel ) {
    elem = rel->entity();
    if ( elem->owner_rank() != p_rank ) { elem = NULL ; }
  }

  STKUNIT_ASSERT( elem != NULL );

  unsigned donated_nodes = 0 ;

  // Only process #0 donates an element and its owned nodes:
  if ( 0 == p_rank ) {
    EntityProc entry ;
    entry.first = elem ;
    entry.second = node->sharing()[0].proc ;
    change.push_back( entry );
    for ( PairIterRelation
          rel = elem->relations(0) ; ! rel.empty() ; ++rel ) {
      if ( rel->entity()->owner_rank() == p_rank ) {
        entry.first = rel->entity();
        change.push_back( entry );
        ++donated_nodes ;
      }
    }
  }

  STKUNIT_ASSERT( mesh.modification_begin() );
  mesh.change_entity_owner( change );
  STKUNIT_ASSERT( UnitTestBulkData::modification_end( mesh , aura ) );

  count_entities( select_owned , mesh , after_count );

  if ( 0 == p_rank ) {
    STKUNIT_ASSERT_EQUAL( before_count[3] - 1 , after_count[3] );
    STKUNIT_ASSERT_EQUAL( before_count[0] - donated_nodes, after_count[0] );
  }
}

void donate_all_shared_nodes( BulkData & mesh , bool aura )
{
  const unsigned p_rank = mesh.parallel_rank();

  const Selector select_used = mesh.mesh_meta_data().locally_owned_part() |
                               mesh.mesh_meta_data().globally_shared_part() ;

  std::vector<unsigned> before_count ;
  std::vector<unsigned> after_count ;

  count_entities( select_used , mesh , before_count );

  // Donate owned shared nodes to first sharing process.

  const std::vector<Entity*> & entity_comm = mesh.entity_comm();

  STKUNIT_ASSERT( ! entity_comm.empty() );

  std::vector<EntityProc> change ;

  for ( std::vector<Entity*>::const_iterator
        i =  entity_comm.begin() ;
        i != entity_comm.end() &&
        (**i).entity_rank() == BaseEntityRank ; ++i ) {
    Entity * const node = *i ;
    const PairIterEntityComm ec = node->sharing();

    if ( node->owner_rank() == p_rank && ! ec.empty() ) {
      change.push_back( EntityProc( node , ec->proc ) );
    }
  }

  STKUNIT_ASSERT( mesh.modification_begin() );
  mesh.change_entity_owner( change );
  STKUNIT_ASSERT( UnitTestBulkData::modification_end( mesh , aura ) );

  count_entities( select_used , mesh , after_count );

  STKUNIT_ASSERT( 3 <= after_count.size() );
  STKUNIT_ASSERT_EQUAL( before_count[0] , after_count[0] );
  STKUNIT_ASSERT_EQUAL( before_count[1] , after_count[1] );
  STKUNIT_ASSERT_EQUAL( before_count[2] , after_count[2] );
  STKUNIT_ASSERT_EQUAL( before_count[3] , after_count[3] );
}

}

void UnitTestBulkData::testChangeOwner_box( ParallelMachine pm )
{
  const int root_box[3][2] = { { 0 , 4 } , { 0 , 5 } , { 0 , 6 } };
  int local_box[3][2] = { { 0 , 0 } , { 0 , 0 } , { 0 , 0 } };

  const unsigned p_rank = parallel_machine_rank( pm );
  const unsigned p_size = parallel_machine_size( pm );

  const int spatial_dimension = 3;
  MetaData meta( TopologicalMetaData::entity_rank_names(spatial_dimension) );

  meta.commit();

  const Selector select_owned( meta.locally_owned_part() );
  const Selector select_used = meta.locally_owned_part() |
                               meta.globally_shared_part() ;
  const Selector select_all(   meta.universal_part() );

  //------------------------------
  {
    BulkData bulk( meta , pm , 100 );
    bulk.modification_begin();
    generate_boxes( bulk , false /* no aura */ , root_box , local_box );

    if ( 1 < p_size ) {
      donate_one_element( bulk , false /* no aura */ );
    }
  }

  if ( 1 < p_size ) {
    BulkData bulk( meta , pm , 100 );
    bulk.modification_begin();

    generate_boxes( bulk , false /* no aura */ , root_box , local_box );

    donate_all_shared_nodes( bulk , false /* no aura */ );
  }
  //------------------------------
  if ( 1 < p_size ) {
    BulkData bulk( meta , pm , 100 );
    bulk.modification_begin();

    generate_boxes( bulk , false /* no aura */ , root_box , local_box );

    donate_one_element( bulk , false /* no aura */ );
  }
  //------------------------------
  // Introduce ghosts:
  if ( 1 < p_size ) {
    BulkData bulk( meta , pm , 100 );
    bulk.modification_begin();

    generate_boxes( bulk , true /* aura */ , root_box , local_box );

    std::vector<unsigned> used_count ;
    std::vector<unsigned> all_count ;

    count_entities( select_all , bulk , all_count );
    count_entities( select_used , bulk , used_count );

    STKUNIT_ASSERT( used_count[0] < all_count[0] );
    STKUNIT_ASSERT( used_count[3] < all_count[3] );

    donate_all_shared_nodes( bulk , false /* don't regenerate aura */ );

    count_entities( select_all , bulk , all_count );
    count_entities( select_used , bulk , used_count );

    STKUNIT_ASSERT_EQUAL( used_count[0] , all_count[0] );
    STKUNIT_ASSERT_EQUAL( used_count[3] , all_count[3] );
  }
  //------------------------------

  std::cout << std::endl
            << "P" << p_rank
            << ": UnitTestBulkData::testChangeOwner_box( NP = "
            << p_size << " ) SUCCESSFULL" << std::endl ;
}

void UnitTestBulkData::testModifyPropagation( ParallelMachine pm )
{
  const unsigned nPerProc = 2;
  const unsigned p_size = parallel_machine_size( pm );

  // this test only needs to be run w/ one processor
  if (p_size > 1) return;

  // Make a ring_mesh and add an extra part
  UnitTestRingMeshFixture ring_mesh( pm , nPerProc, false /* don't use edge parts */);
  TopologicalMetaData & top_data = ring_mesh.m_top_data;
  stk::mesh::Part& special_part =
    ring_mesh.m_meta_data.declare_part( "special_node_part", stk::mesh::BaseEntityRank );
  ring_mesh.m_meta_data.commit();
  ring_mesh.generate_mesh();

  // grab the first edge
  stk::mesh::EntityVector edges;
  stk::mesh::get_entities( ring_mesh.m_bulk_data, top_data.edge_rank, edges );
  stk::mesh::Entity& edge = *( edges.front() );

  // get one of the nodes related to this edge
  PairIterRelation node_relations = edge.relations( stk::mesh::BaseEntityRank );
  STKUNIT_ASSERT( !node_relations.empty() );
  stk::mesh::Entity& node = *( node_relations.front().entity());
  STKUNIT_ASSERT_EQUAL( node.entity_rank(), (unsigned) stk::mesh::BaseEntityRank );

  // make a modification to the node by changing its parts
  ring_mesh.m_bulk_data.modification_begin();
  stk::mesh::PartVector parts;
  parts.push_back( &special_part );
  ring_mesh.m_bulk_data.change_entity_parts( node, parts );

  // check that the node AND it's edge are marked as modified
  STKUNIT_ASSERT_EQUAL ( node.log_query(), stk::mesh::EntityLogModified );
  STKUNIT_ASSERT_EQUAL ( edge.log_query(), stk::mesh::EntityLogModified );

  STKUNIT_ASSERT ( ring_mesh.m_bulk_data.modification_end() );
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

} // namespace mesh
} // namespace stk


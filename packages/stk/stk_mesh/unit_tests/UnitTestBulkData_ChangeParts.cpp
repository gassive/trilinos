/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/


#include <iostream>
#include <sstream>

#include <stk_util/unit_test_support/stk_utest_macros.hpp>

#include <stk_util/parallel/Parallel.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/GetEntities.hpp>
#include <stk_mesh/base/Comm.hpp>
#include <stk_mesh/fem/EntityRanks.hpp>

#include <unit_tests/UnitTestBulkData.hpp>
#include <unit_tests/UnitTestRingMeshFixture.hpp>

//----------------------------------------------------------------------
//----------------------------------------------------------------------

namespace stk {
namespace mesh {

//----------------------------------------------------------------------

void UnitTestBulkData::testChangeParts( ParallelMachine pm )
{
  static const char method[] =
    "stk::mesh::UnitTestBulkData::testChangeParts" ;

  std::cout << std::endl << method << std::endl ;

  const unsigned p_size = parallel_machine_size( pm );
  const unsigned p_rank = parallel_machine_rank( pm );

  if ( 1 < p_size ) return ;

  // Single process, no sharing

  // Meta data with entity ranks [0..9]
  std::vector<std::string> entity_names(10);
  for ( size_t i = 0 ; i < 10 ; ++i ) {
    std::ostringstream name ;
    name << "EntityRank_" << i ;
    entity_names[i] = name.str();
  }

  MetaData meta( entity_names );
  BulkData bulk( meta , pm , 100 );

  Part & part_univ = meta.universal_part();
  Part & part_owns = meta.locally_owned_part();

  Part & part_A_0 = meta.declare_part( std::string("A_0") , 0 );
  Part & part_A_1 = meta.declare_part( std::string("A_1") , 1 );
  Part & part_A_2 = meta.declare_part( std::string("A_2") , 2 );
  Part & part_A_3 = meta.declare_part( std::string("A_3") , 3 );

  Part & part_B_0 = meta.declare_part( std::string("B_0") , 0 );
  // Part & part_B_1 = meta.declare_part( std::string("B_1") , 1 );
  Part & part_B_2 = meta.declare_part( std::string("B_2") , 2 );
  // Part & part_B_3 = meta.declare_part( std::string("B_3") , 3 );

  meta.commit();
  bulk.modification_begin();

  PartVector tmp(1);

  tmp[0] = & part_A_0 ;
  Entity & entity_0_1 = bulk.declare_entity(  0 , 1 , tmp );

  tmp[0] = & part_A_1 ;
  Entity & entity_1_1 = bulk.declare_entity(  1 , 1 , tmp );

  tmp[0] = & part_A_2 ;
  Entity & entity_2_1 = bulk.declare_entity(  2 , 1 , tmp );

  tmp[0] = & part_A_3 ;
  Entity & entity_3_1 = bulk.declare_entity( 3 , 1 , tmp );

  entity_0_1.bucket().supersets( tmp );
  STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
  STKUNIT_ASSERT( tmp[0] == & part_univ );
  STKUNIT_ASSERT( tmp[1] == & part_owns );
  STKUNIT_ASSERT( tmp[2] == & part_A_0 );

  entity_1_1.bucket().supersets( tmp );
  STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
  STKUNIT_ASSERT( tmp[0] == & part_univ );
  STKUNIT_ASSERT( tmp[1] == & part_owns );
  STKUNIT_ASSERT( tmp[2] == & part_A_1 );

  entity_2_1.bucket().supersets( tmp );
  STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
  STKUNIT_ASSERT( tmp[0] == & part_univ );
  STKUNIT_ASSERT( tmp[1] == & part_owns );
  STKUNIT_ASSERT( tmp[2] == & part_A_2 );

  entity_3_1.bucket().supersets( tmp );
  STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
  STKUNIT_ASSERT( tmp[0] == & part_univ );
  STKUNIT_ASSERT( tmp[1] == & part_owns );
  STKUNIT_ASSERT( tmp[2] == & part_A_3 );

  {
    tmp.resize(1);
    tmp[0] = & part_A_0 ;
    bulk.change_entity_parts( entity_0_1 , tmp );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
  }

  { // Add a new part:
    tmp.resize(1);
    tmp[0] = & part_B_0 ;
    bulk.change_entity_parts( entity_0_1 , tmp );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
    STKUNIT_ASSERT( tmp[3] == & part_B_0 );
  }

  { // Remove the part just added:
    tmp.resize(1);
    tmp[0] = & part_B_0 ;
    bulk.change_entity_parts( entity_0_1 , PartVector() , tmp );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
  }

  { // Relationship induced membership:
    bulk.declare_relation( entity_1_1 , entity_0_1 , 0 );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
    STKUNIT_ASSERT( tmp[3] == & part_A_1 );
  }

  { // Remove relationship induced membership:
    bulk.destroy_relation( entity_1_1 , entity_0_1 );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
  }

  { // Add a new part:
    tmp.resize(1);
    tmp[0] = & part_B_2 ;
    bulk.change_entity_parts( entity_2_1 , tmp );
    entity_2_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_2 );
    STKUNIT_ASSERT( tmp[3] == & part_B_2 );
  }

  { // Relationship induced membership:
    bulk.declare_relation( entity_2_1 , entity_0_1 , 0 );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(5) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
    STKUNIT_ASSERT( tmp[3] == & part_A_2 );
    STKUNIT_ASSERT( tmp[4] == & part_B_2 );
  }

  { // Remove relationship induced membership:
    bulk.destroy_relation( entity_2_1 , entity_0_1 );
    entity_0_1.bucket().supersets( tmp );
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
  }

  bulk.modification_end();

  //------------------------------
  // Now the parallel fun.  Existing entities should be shared
  // by all processes since they have the same identifiers.
  // They should also have the same parts.

  entity_0_1.bucket().supersets( tmp );
  if ( entity_0_1.owner_rank() == p_rank ) {
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
  }
  else {
    STKUNIT_ASSERT_EQUAL( size_t(2) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_A_0 );
  }

  entity_2_1.bucket().supersets( tmp );
  if ( entity_2_1.owner_rank() == p_rank ) {
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_2 );
    STKUNIT_ASSERT( tmp[3] == & part_B_2 );
  }
  else {
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_A_2 );
    STKUNIT_ASSERT( tmp[2] == & part_B_2 );
  }

  if (bulk.parallel_size() > 1) {
    STKUNIT_ASSERT_EQUAL( size_t(p_size - 1) , entity_0_1.sharing().size() );
    STKUNIT_ASSERT_EQUAL( size_t(p_size - 1) , entity_1_1.sharing().size() );
    STKUNIT_ASSERT_EQUAL( size_t(p_size - 1) , entity_2_1.sharing().size() );
    STKUNIT_ASSERT_EQUAL( size_t(p_size - 1) , entity_3_1.sharing().size() );
  }

  bulk.modification_begin();

  // Add a new part on the owning process:

  int ok_to_modify = entity_0_1.owner_rank() == p_rank ;

  try {
    tmp.resize(1);
    tmp[0] = & part_B_0 ;
    bulk.change_entity_parts( entity_0_1 , tmp );
    STKUNIT_ASSERT( ok_to_modify );
  }
  catch( const std::exception & x ) {
    STKUNIT_ASSERT( ! ok_to_modify );
  }

  entity_0_1.bucket().supersets( tmp );
  if ( entity_0_1.owner_rank() == p_rank ) {
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
    STKUNIT_ASSERT( tmp[3] == & part_B_0 );
  }
  else {
    STKUNIT_ASSERT_EQUAL( size_t(2) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_A_0 );
  }

  bulk.modification_end();

  entity_0_1.bucket().supersets( tmp );
  if ( entity_0_1.owner_rank() == p_rank ) {
    STKUNIT_ASSERT_EQUAL( size_t(4) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_owns );
    STKUNIT_ASSERT( tmp[2] == & part_A_0 );
    STKUNIT_ASSERT( tmp[3] == & part_B_0 );
  }
  else {
    STKUNIT_ASSERT_EQUAL( size_t(3) , tmp.size() );
    STKUNIT_ASSERT( tmp[0] == & part_univ );
    STKUNIT_ASSERT( tmp[1] == & part_A_0 );
    STKUNIT_ASSERT( tmp[2] == & part_B_0 );
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

void UnitTestBulkData::testChangeParts_loop( ParallelMachine pm )
{
  enum { nPerProc = 10 };
  const unsigned p_rank = parallel_machine_rank( pm );
  const unsigned p_size = parallel_machine_size( pm );
  const unsigned nLocalNode = nPerProc + ( 1 < p_size ? 1 : 0 );
  const unsigned nLocalEdge = nPerProc ;

  UnitTestRingMeshFixture ring_mesh( pm , nPerProc , true /* generate parts */ );
  ring_mesh.m_meta_data.commit();
  ring_mesh.generate_mesh( false /* no aura */ );

  Part & part_owns = ring_mesh.m_meta_data.locally_owned_part();
  Part & part_univ = ring_mesh.m_meta_data.universal_part();

  Selector select_owned( ring_mesh.m_meta_data.locally_owned_part() );
  Selector select_used = select_owned | ring_mesh.m_meta_data.globally_shared_part();
  Selector select_all(   ring_mesh.m_meta_data.universal_part() );

  std::vector<unsigned> local_count ;

  for ( unsigned i = 0 ; i < nLocalEdge ; ++i ) {
    const unsigned n = i + nPerProc * p_rank ;
    Entity * const edge = ring_mesh.m_bulk_data.get_entity( 1 , ring_mesh.m_edge_ids[n] );
    STKUNIT_ASSERT( edge != NULL );
    STKUNIT_ASSERT( edge->bucket().member( part_univ ) );
    STKUNIT_ASSERT( edge->bucket().member( part_owns ) );
    STKUNIT_ASSERT( edge->bucket().member( * ring_mesh.m_edge_parts[ n % ring_mesh.m_edge_parts.size() ] ) );
  }

  for ( unsigned i = 0 ; i < nLocalNode ; ++i ) {
    const unsigned n = ( i + nPerProc * p_rank ) % ring_mesh.m_node_ids.size();
    const unsigned e0 = n ;
    const unsigned e1 = ( n + ring_mesh.m_edge_ids.size() - 1 ) % ring_mesh.m_edge_ids.size();
    const unsigned ns = ring_mesh.m_edge_parts.size();
    const unsigned n0 = e0 % ns ;
    const unsigned n1 = e1 % ns ;
    Part * const epart_0 = ring_mesh.m_edge_parts[ n0 < n1 ? n0 : n1 ];
    Part * const epart_1 = ring_mesh.m_edge_parts[ n0 < n1 ? n1 : n0 ];

    Entity * const node = ring_mesh.m_bulk_data.get_entity( 0 , ring_mesh.m_node_ids[n] );
    STKUNIT_ASSERT( node != NULL );
    if ( node->owner_rank() == p_rank ) {
      STKUNIT_ASSERT( node->bucket().member( part_univ ) );
      STKUNIT_ASSERT( node->bucket().member( part_owns ) );
      STKUNIT_ASSERT( node->bucket().member( *epart_0 ) );
      STKUNIT_ASSERT( node->bucket().member( *epart_1 ) );
    }
    else {
      STKUNIT_ASSERT( node->bucket().member( part_univ ) );
      STKUNIT_ASSERT( ! node->bucket().member( part_owns ) );
      STKUNIT_ASSERT( node->bucket().member( * epart_0 ) );
      STKUNIT_ASSERT( node->bucket().member( * epart_1 ) );
    }
  }

  ring_mesh.m_bulk_data.modification_begin();

  if ( 0 == p_rank ) {

    for ( unsigned i = 0 ; i < nLocalEdge ; ++i ) {
      const unsigned n = i + nPerProc * p_rank ;

      PartVector add(1); add[0] = & ring_mesh.m_edge_part_extra ;
      PartVector rem(1); rem[0] = ring_mesh.m_edge_parts[ n % ring_mesh.m_edge_parts.size() ];

      Entity * const edge = ring_mesh.m_bulk_data.get_entity( 1 , ring_mesh.m_edge_ids[n] );
      ring_mesh.m_bulk_data.change_entity_parts( *edge , add , rem );
      STKUNIT_ASSERT( edge->bucket().member( part_univ ) );
      STKUNIT_ASSERT( edge->bucket().member( part_owns ) );
      STKUNIT_ASSERT( edge->bucket().member(ring_mesh.m_edge_part_extra ) );
    }
  }

  ring_mesh.m_bulk_data.modification_end();

  for ( unsigned i = 0 ; i < nLocalNode ; ++i ) {
    const unsigned n = ( i + nPerProc * p_rank ) % ring_mesh.m_node_ids.size();
    const unsigned e0 = n ;
    const unsigned e1 = ( n + ring_mesh.m_edge_ids.size() - 1 ) % ring_mesh.m_edge_ids.size();
    const unsigned ns = ring_mesh.m_edge_parts.size();
    const unsigned n0 = e0 % ns ;
    const unsigned n1 = e1 % ns ;
    Part * ep_0 = e0 < nLocalEdge ? & ring_mesh.m_edge_part_extra : ring_mesh.m_edge_parts[n0] ;
    Part * ep_1 = e1 < nLocalEdge ? & ring_mesh.m_edge_part_extra : ring_mesh.m_edge_parts[n1] ;

    Part * epart_0 = ep_0->mesh_meta_data_ordinal() < ep_1->mesh_meta_data_ordinal() ? ep_0 : ep_1 ;
    Part * epart_1 = ep_0->mesh_meta_data_ordinal() < ep_1->mesh_meta_data_ordinal() ? ep_1 : ep_0 ;

    Entity * const node = ring_mesh.m_bulk_data.get_entity( 0 , ring_mesh.m_node_ids[n] );
    STKUNIT_ASSERT( node != NULL );
    if ( node->owner_rank() == p_rank ) {
      STKUNIT_ASSERT( node->bucket().member( part_owns ) );
    }
    else {
      STKUNIT_ASSERT( ! node->bucket().member( part_owns ) );
    }

    STKUNIT_ASSERT( node->bucket().member( part_univ ) );
    STKUNIT_ASSERT( node->bucket().member( *epart_0 ) );
    STKUNIT_ASSERT( node->bucket().member( *epart_1 ) );
  }
}

} // namespace mesh
} // namespace stk


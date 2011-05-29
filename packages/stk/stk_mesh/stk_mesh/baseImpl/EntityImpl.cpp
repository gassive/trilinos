/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/

#include <algorithm>
#include <sstream>
#include <stdexcept>
#include <cassert>

#include <stk_mesh/base/Ghosting.hpp>

#include <stk_mesh/baseImpl/EntityImpl.hpp>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/MetaData.hpp>

namespace stk {
namespace mesh {
namespace impl {

EntityImpl::~EntityImpl()
{
}


EntityImpl::EntityImpl( const EntityKey & arg_key )
  : m_key(arg_key),
    m_relation(),
    m_comm(),
    m_bucket( NULL ),
    m_bucket_ord(0),
    m_owner_rank(0),
    m_sync_count(0),
    m_mod_log( EntityLogCreated )
{
}


PairIterRelation EntityImpl::relations( unsigned rank ) const
{
  std::vector<Relation>::const_iterator i = m_relation.begin();
  std::vector<Relation>::const_iterator e = m_relation.end();

  if ( rank ) {
    const Relation::raw_attr_type lo_attr = Relation::attribute( rank , 0 );
    i = std::lower_bound( i , e , lo_attr , LessRelation() );
  }

  const Relation::raw_attr_type hi_attr = Relation::attribute( rank + 1 , 0 );
  e = std::lower_bound( i , e , hi_attr , LessRelation() );

  return PairIterRelation( i , e );
}


PairIterEntityComm EntityImpl::sharing() const
{
  EntityCommInfoVector::const_iterator i = m_comm.begin();
  EntityCommInfoVector::const_iterator e = m_comm.end();

  e = std::lower_bound( i , e , EntityCommInfo(1,0) );

  return PairIterEntityComm( i , e );
}


PairIterEntityComm EntityImpl::comm( const Ghosting & sub ) const
{
  typedef std::vector< EntityCommInfo > EntityComm ;

  const EntityCommInfo s_begin( sub.ordinal() ,     0 );
  const EntityCommInfo s_end(   sub.ordinal() + 1 , 0 );

  EntityComm::const_iterator i = m_comm.begin();
  EntityComm::const_iterator e = m_comm.end();

  i = std::lower_bound( i , e , s_begin );
  e = std::lower_bound( i , e , s_end );

  return PairIterEntityComm( i , e );
}


bool EntityImpl::insert( const EntityCommInfo & val )
{
  std::vector< EntityCommInfo >::iterator i =
    std::lower_bound( m_comm.begin() , m_comm.end() , val );

  const bool result = i == m_comm.end() || val != *i ;

  if ( result ) {
    m_comm.insert( i , val );
  }

  return result ;
}


bool EntityImpl::erase( const EntityCommInfo & val )
{
  std::vector< EntityCommInfo >::iterator i =
    std::lower_bound( m_comm.begin() , m_comm.end() , val );

  const bool result = i != m_comm.end() && val == *i ;

  if ( result ) {
    m_comm.erase( i );
  }

  return result ;
}


bool EntityImpl::erase( const Ghosting & ghost )
{
  typedef std::vector< EntityCommInfo > EntityComm ;

  const EntityCommInfo s_begin( ghost.ordinal() ,     0 );
  const EntityCommInfo s_end(   ghost.ordinal() + 1 , 0 );

  EntityComm::iterator i = m_comm.begin();
  EntityComm::iterator e = m_comm.end();

  i = std::lower_bound( i , e , s_begin );
  e = std::lower_bound( i , e , s_end );

  const bool result = i != e ;

  if ( result ) {
    m_comm.erase( i , e );
  }

  return result ;
}


void EntityImpl::comm_clear_ghosting()
{
  std::vector< EntityCommInfo >::iterator j = m_comm.begin();
  while ( j != m_comm.end() && j->ghost_id == 0 ) { ++j ; }
  m_comm.erase( j , m_comm.end() );
}


void EntityImpl::comm_clear() {
  m_comm.clear();
}

bool EntityImpl::marked_for_destruction() const {
  // The original implementation of this method checked bucket capacity. In
  // order to ensure that the addition of EntityLogDeleted does not change
  // behavior, we put error check here.
  if ( (bucket().capacity() == 0) != (m_mod_log == EntityLogDeleted) ) {
    std::ostringstream msg;
    msg << "Inconsistent destruction state; "
        << "destroyed entities should be in the nil bucket and vice versa.\n"
        << "Problem is with entity: "
        << print_entity_key( msg, bucket().mesh().mesh_meta_data(), key() )
        << "\n"
        << "Was in nil bucket: " << (bucket().capacity() == 0) << ", "
        << "was in destroyed state: " << (m_mod_log == EntityLogDeleted);
    throw std::runtime_error( msg.str() );
  }
  return m_mod_log == EntityLogDeleted;
}

namespace {

bool is_degenerate_relation ( const Relation &r1 , const Relation &r2 )
{
  return r1.attribute() == r2.attribute() && r1.entity() != r2.entity() ;
}

}

void EntityImpl::log_resurrect()
{
  if ( EntityLogDeleted != m_mod_log ) {
    std::ostringstream msg;
    msg << "Trying to resurrect non-deleted entity: "
        << print_entity_key( msg, bucket().mesh().mesh_meta_data(), key() );
    throw std::runtime_error( msg.str() );
  }
  m_mod_log = EntityLogModified;
  m_bucket = NULL;
}

void EntityImpl::log_created_parallel_copy()
{
  if ( EntityLogCreated == m_mod_log ) {
    m_mod_log = EntityLogModified ;
  }
}

bool EntityImpl::destroy_relation( Entity& e_to )
{
  bool destroyed_relations = false;
  for ( std::vector<Relation>::iterator i = m_relation.end() ;
        i != m_relation.begin() ; ) {
    --i ;
    if ( i->entity() == & e_to ) {
      i = m_relation.erase( i );
      destroyed_relations = true;
    }
  }
  return destroyed_relations;
}

bool EntityImpl::declare_relation( Entity & e_to,
                                   const unsigned local_id,
                                   unsigned sync_count,
                                   bool is_converse )
{
  const MetaData & meta_data = bucket().mesh().mesh_meta_data();

  static const char method[] = "stk::mesh::impl::EntityImpl::declare_relation";

  const Relation new_relation( e_to , local_id );

  const std::vector<Relation>::iterator fe = m_relation.end();
        std::vector<Relation>::iterator fi = m_relation.begin();

  fi = std::lower_bound( fi , fe , new_relation , LessRelation() );

  // The ordering of the Relations allows for two situations that do
  // not arise often in meshes.  The first situation is 2 relations between
  // e_from and e_to with the same kind but different local_ids.  This
  // can happen if, for example, a triangle should be used as a quad.  In
  // this case, one node of the triangle must be two different local nodes of
  // the quad.  This situation is a valid state of mesh entities.

  // The second situation involves malformed stencils.  Given e_from, e_to1,
  // and e_to2, e_to1 and eto2 can share a relation with e_from with the same
  // kind and local_id.  This can arise, for instance, if an edge has three
  // nodes.  The local_id 1 of the edge may point to two different nodes.
  // This situation is disallowed in the mesh.  We now check for it.

  bool found_degenerate_relation = false;
  EntityKey  degenerate_key;
  if ( fi != fe )
  {
    bool  downstream = fi->entity_rank() < entity_rank();
    if ( is_degenerate_relation ( new_relation , *fi ) && downstream )
    {
      found_degenerate_relation = true;
      degenerate_key = fi->entity()->key();
    }
  }
  if ( fi != m_relation.begin() )
  {
    --fi;
    bool  downstream = fi->entity_rank() < entity_rank();
    if ( is_degenerate_relation ( new_relation , *fi ) && downstream )
    {
      found_degenerate_relation = true;
      degenerate_key = fi->entity()->key();
    }
    ++fi;
  }
  if ( found_degenerate_relation && !is_converse )
  {
    std::ostringstream msg ;
    msg << method << "( from " ;
    print_entity_key( msg , meta_data, key() );
    msg << " , to " ;
    print_entity_key( msg , meta_data, e_to.key() );
    msg << " , id " << local_id ;
    msg << " ) FAILED ";
    msg << " Relation already exists to " ;
    print_entity_key( msg , meta_data, degenerate_key );
    throw std::runtime_error( msg.str() );
  }

  // If the relation is not degenerate, we add it

  if ( ( !is_converse && (fe == fi ||
                          new_relation.attribute() != fi->attribute() ) ) ||
       (is_converse && (fe == fi || new_relation != *fi ) ) ) {

    fi = m_relation.insert( fi , new_relation );

    set_sync_count( sync_count );

    return true;
  }
  else {
    return false;
  }
}

} // namespace impl
} // namespace mesh
} // namespace stk


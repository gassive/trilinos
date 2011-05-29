/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/BulkData.hpp>
#include <stk_mesh/base/Entity.hpp>
#include <stk_mesh/base/Bucket.hpp>
#include <stk_mesh/base/Relation.hpp>
#include <stk_mesh/base/FieldData.hpp>

namespace stk {
namespace mesh {

//----------------------------------------------------------------------

std::ostream &
operator << ( std::ostream & s , const Relation & rel )
{
  Entity * const e = rel.entity();

  if ( e ) {
    const MetaData & meta_data = e->bucket().mesh().mesh_meta_data();

    s << meta_data.entity_rank_name( rel.entity_rank() );
    s << "[" << rel.identifier() << "]->" ;
    print_entity_key( s , meta_data , e->key() );
  }
  else {
    s << rel.entity_rank();
    s << "[" << rel.identifier() << "]->NULL" ;
  }

  return s ;
}

//----------------------------------------------------------------------

Relation::raw_attr_type
Relation::attribute( unsigned rank , unsigned id )
{
  if ( id_mask < id ) {
    std::ostringstream msg ;
    msg << "stk::mesh::Relation::attribute( " ;
    msg << rank << " , " << id << " ) FAILED : " ;
    msg << id << " > " << id_mask ;
    throw std::runtime_error( msg.str() );
  }

  return ( raw_attr_type(rank) << rank_shift ) | id ;
}

Relation::Relation( Entity & entity , unsigned identifier )
  : m_attr( Relation::attribute( entity.entity_rank() , identifier ) ),
    m_entity( & entity )
{}

Relation::Relation( Relation::raw_attr_type attr , Entity & entity )
  : m_attr( attr ), m_entity( & entity )
{
  if ( entity_rank() != entity.entity_rank() ) {
    std::ostringstream msg ;
    msg << "stk::mesh::Relation::Relation( "  ;
    msg << *this ;
    msg << " ) INCOMPATIBLE ARGUMENTS" ;
    throw std::invalid_argument( msg.str() );
  }
}

bool Relation::operator < ( const Relation & r ) const
{
  bool result = false;

  if ( m_attr.value != r.m_attr.value ) {
    result = m_attr.value < r.m_attr.value ;
  }
  else {
    const EntityKey lhs = m_entity   ? m_entity->key()   : EntityKey() ;
    const EntityKey rhs = r.m_entity ? r.m_entity->key() : EntityKey() ;
    result = lhs < rhs ;
  }
  return result ;
}

//----------------------------------------------------------------------

namespace {

void get_entities_through_relations(
  PairIterRelation rel ,
  const std::vector<Entity*>::const_iterator i_beg ,
  const std::vector<Entity*>::const_iterator i_end ,
  std::vector<Entity*> & entities_related )
{
  for ( ; rel.first != rel.second ; ++rel.first ) {

    // Do all input entities have a relation to this entity ?

    Entity * const e = rel.first->entity();

    std::vector<Entity*>::const_iterator i = i_beg ;

    for ( ; i != i_end ; ++i ) {
      PairIterRelation r = (*i)->relations();
      while ( r.first != r.second && e != r.first->entity() ) {
        ++r.first ;
      }
      if ( r.first == r.second ) { break ; }
    }

    if ( i == i_end ) {
      entities_related.push_back( e );
    }
  }
}

}

void get_entities_through_relations(
  const std::vector<Entity*> & entities ,
        std::vector<Entity*> & entities_related )
{
  entities_related.clear();

  if ( ! entities.empty() ) {
          std::vector<Entity*>::const_iterator i = entities.begin();
    const std::vector<Entity*>::const_iterator j = entities.end();

    PairIterRelation rel = (*i)->relations(); ++i ;

    get_entities_through_relations( rel , i , j , entities_related );
  }
}

void get_entities_through_relations(
  const std::vector<Entity*> & entities ,
        unsigned               entities_related_rank ,
        std::vector<Entity*> & entities_related )
{
  entities_related.clear();

  if ( ! entities.empty() ) {
          std::vector<Entity*>::const_iterator i = entities.begin();
    const std::vector<Entity*>::const_iterator j = entities.end();

    PairIterRelation rel = (*i)->relations( entities_related_rank ); ++i ;

    get_entities_through_relations( rel , i , j , entities_related );
  }
}

//----------------------------------------------------------------------

/** \brief  Query if a member entity of the given entity type
 *          has an induced membership.
 */
bool membership_is_induced( const Part & part , unsigned entity_rank )
{
  const MetaData & meta = part.mesh_meta_data();

  const bool induced_by_type =
     entity_rank < part.primary_entity_rank() &&
                   part.primary_entity_rank() < meta.entity_rank_count() ;

  const bool induced_by_stencil =
    ! part.relations().empty() &&
      part.relations().begin()->m_target == & part ;

  return induced_by_type || induced_by_stencil ;
}

//----------------------------------------------------------------------

void induced_part_membership( Part & part ,
                              unsigned entity_rank_from ,
                              unsigned entity_rank_to ,
                              unsigned relation_identifier ,
                              PartVector & induced_parts )
{
  if ( entity_rank_to < entity_rank_from &&
       part.primary_entity_rank() == entity_rank_from ) {

    // Direct relationship:

    insert( induced_parts , part );

    // Stencil relationship where 'part' is the root:
    // The 'target' should not have subsets or supersets.

    const std::vector<PartRelation> & part_rel = part.relations();

    for ( std::vector<PartRelation>::const_iterator
          j = part_rel.begin() ; j != part_rel.end() ; ++j ) {

      if ( & part == j->m_root &&
           0 <= (* j->m_function)( entity_rank_from , entity_rank_to ,
                                   relation_identifier ) ) {
        insert( induced_parts , * j->m_target );
      }
    }
  }
}

//----------------------------------------------------------------------
//  What are this entity's part memberships that can be deduced from
//  this entity's relationship.  Can only trust 'entity_from' to be
//  accurate if it is owned by the local process.

void induced_part_membership( const Entity     & entity_from ,
                              const PartVector & omit ,
                                    unsigned     entity_rank_to ,
                                    unsigned     relation_identifier ,
                                    PartVector & entity_to_parts )
{
  const Bucket   & bucket_from    = entity_from.bucket();
  const BulkData & mesh           = bucket_from.mesh();
  const unsigned local_proc_rank  = mesh.parallel_rank();
  const unsigned entity_rank_from = entity_from.entity_rank();

  if ( entity_rank_to < entity_rank_from &&
       local_proc_rank == entity_from.owner_rank() ) {
    const MetaData   & meta        = mesh.mesh_meta_data();
    const PartVector & all_parts   = meta.get_parts();

    const std::pair<const unsigned *, const unsigned *>
      bucket_superset_ordinals = bucket_from.superset_part_ordinals();

    // Contributions of the 'from' entity:

    for ( const unsigned * i = bucket_superset_ordinals.first ;
                           i < bucket_superset_ordinals.second ; ++i ) {

      Part & part = * all_parts[*i] ;

      if ( ! contain( omit , part ) ) {
        induced_part_membership( part,
                                 entity_rank_from ,
                                 entity_rank_to ,
                                 relation_identifier ,
                                 entity_to_parts );
      }
    }
  }
}

//----------------------------------------------------------------------

void induced_part_membership( const Entity     & entity ,
                              const PartVector & omit ,
                                    PartVector & induced )
{
  for ( PairIterRelation
        rel = entity.relations() ; ! rel.empty() ; ++rel ) {

    induced_part_membership( * rel->entity() , omit ,
                             entity.entity_rank() ,
                             rel->identifier() ,
                             induced );
  }
}

//----------------------------------------------------------------------

} // namespace mesh
} // namespace stk


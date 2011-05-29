/*
//@HEADER
// ************************************************************************
//
//                Shards : Shared Discretization Tools
//                 Copyright 2008 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Carter Edwards (hcedwar@sandia.gov),
//                    Pavel Bochev (pbboche@sandia.gov), or
//                    Denis Ridzal (dridzal@sandia.gov).
//
// ************************************************************************
//@HEADER
*/

//#define HAVE_SHARDS_DEBUG

#include <stdexcept>
#include <sstream>
#include <Shards_CellTopology.hpp>
#include <Shards_BasicTopologies.hpp>
#include <iostream>

namespace shards {

typedef CellTopologyData_Subcell Subcell ;

class CellTopologyPrivate {
public:
  unsigned                m_count_ref ;
  CellTopologyData        m_cell ;
  std::string             m_name ;
  std::vector< Subcell >  m_subcell ;
  std::vector< unsigned > m_node_map ;

  /** \brief  1D topology */
  CellTopologyPrivate( const std::string & name , const unsigned nodeCount );

  /** \brief  2D topology */
  CellTopologyPrivate(
    const std::string                             & name,
    const unsigned                                  vertexCount,
    const unsigned                                  nodeCount,
    const std::vector< const CellTopologyData * > & edges ,
    const std::vector< unsigned >                 & edge_node_map ,
    const CellTopologyData                        * base );

  /** \brief  3D topology */
  CellTopologyPrivate(
    const std::string                             & name,
    const unsigned                                  vertexCount,
    const unsigned                                  nodeCount,
    const std::vector< const CellTopologyData * > & edges ,
    const std::vector< unsigned >                 & edge_node_map ,
    const std::vector< const CellTopologyData * > & faces ,
    const std::vector< unsigned >                 & face_node_map ,
    const CellTopologyData                        * base );

private:
  CellTopologyPrivate();
  CellTopologyPrivate( const CellTopologyPrivate & );
  CellTopologyPrivate & operator = ( const CellTopologyPrivate & );
};

//----------------------------------------------------------------------
// Manage assignment and destruction of allocated 'm_owned' member.

CellTopology::~CellTopology()
{
  deleteOwned();
}

CellTopology::CellTopology()
  : m_cell( NULL ), m_owned( NULL )
{}

CellTopology::CellTopology( const CellTopology & right )
  : m_cell( NULL ), m_owned( NULL )
{
  // Use assignment operator for proper memory management of 'm_owned'
  operator=( right );
}

CellTopology & CellTopology::operator = ( const CellTopology & right )
{
  if ( this != & right ) {
  { // Proper memory management of shared and allocated 'm_owned'
    deleteOwned();
    m_owned = right.m_owned ;
    if ( m_owned ) { ++( m_owned->m_count_ref ); }
  }
  m_cell = right.m_cell ;
}
  return *this ;
}

void CellTopology::deleteOwned()
{
  if ( m_owned ) {
    if ( m_owned->m_count_ref ) {
      --( m_owned->m_count_ref );
    }
    else {
      delete m_owned ;
    }
    m_owned = NULL ;
  }
}

//----------------------------------------------------------------------

CellTopology::CellTopology(
  const std::string & name,
  const unsigned      node_count )
  : m_cell(NULL), m_owned(NULL)
{
  m_owned = new CellTopologyPrivate( name , node_count );
  m_cell  = & m_owned->m_cell ;
}



CellTopology::CellTopology(
  const std::string                             & name,
  const unsigned                                  vertex_count ,
  const unsigned                                  node_count,
  const std::vector< const CellTopologyData * > & edges ,
  const std::vector< unsigned >                 & edge_node_map ,
  const CellTopologyData                        * base )
  : m_cell(NULL), m_owned(NULL)
{
  m_owned = new CellTopologyPrivate( name , 
                                     vertex_count , node_count ,
                                     edges , edge_node_map , 
                                     base );
  m_cell  = & m_owned->m_cell ;
}



CellTopology::CellTopology( const std::string                             & name,
                            const unsigned                                  vertex_count,
                            const unsigned                                  node_count,
                            const std::vector< const CellTopologyData * > & edges ,
                            const std::vector< unsigned >                 & edge_node_map ,
                            const std::vector< const CellTopologyData * > & faces ,
                            const std::vector< unsigned >                 & face_node_map ,
                            const CellTopologyData                        * base)
: m_cell(NULL), m_owned(NULL)
{
  m_owned = new CellTopologyPrivate( name , 
                                     vertex_count , node_count ,
                                     edges , edge_node_map , 
                                     faces , face_node_map , 
                                     base );
  m_cell  = & m_owned -> m_cell ;
  
}


//----------------------------------------------------------------------

void CellTopology::requireCell() const
{
  if ( m_cell == NULL || m_cell->base == NULL ) {
    std::string msg ;
    msg.append( "shards::CellTopology::requireCell() : FAILED " );
    if ( m_cell == NULL ) {
      msg.append( "is NULL" );
    }
    else {
      msg.append("'");
      msg.append( m_cell->name );
      msg.append("' has NULL base");
    }
    msg.append( " ) FAILED" );
    throw std::runtime_error( msg );
  }
}

void CellTopology::requireDimension( const unsigned subcellDim ) const
{
  if ( 3 < subcellDim ) {
    std::ostringstream msg ;
    msg << "shards::CellTopology::requireDimension( ERROR: dim = "
        << subcellDim << " > 3 )" ;
    throw std::invalid_argument( msg.str() );
  }
}

void CellTopology::requireSubcell( const unsigned subcellDim ,
                                   const unsigned subcellOrd ) const
{
  if ( m_cell->subcell_count[ subcellDim ] <= subcellOrd ) {
    std::ostringstream msg ;
    msg << "shards::CellTopology::requireSubcell( dim = "
        << subcellDim << " , ERROR: ord = " << subcellOrd
        << " > '" << m_cell->name
        << "'.subcell_count[" << subcellDim
        << "] = " << m_cell->subcell_count[ subcellDim ]
        << " )" ;
    throw std::invalid_argument( msg.str() );
  }
}

void CellTopology::requireNodeMap( const unsigned subcellDim ,
                                   const unsigned subcellOrd ,
                                   const unsigned nodeOrd ) const
{
  const unsigned n =
    m_cell->subcell[subcellDim][subcellOrd].topology->node_count ;

  if ( n <= nodeOrd ) {
    std::ostringstream msg ;
    msg << "shards::CellTopology::requireNodeMap( " 
        << subcellDim << " , "
        << subcellOrd
        << " , ERROR: " << nodeOrd << " >= '"
        << m_cell->name 
        << "'.subcell[" << subcellDim
        << "][" << subcellOrd
        << "].topology->node_count = "
        << n << " )" ;
    throw std::invalid_argument( msg.str() );
  }
}

void CellTopology::requireNodePermutation( const unsigned permutationOrd ,
                                           const unsigned nodeOrd ) const
{
  const bool bad_p = m_cell->permutation_count <= permutationOrd ;
  const bool bad_n = m_cell->node_count        <= nodeOrd ;
  if ( bad_p || bad_n ) {
    std::ostringstream msg ;
    msg << "shards::CellTopology::requireNodePermutation( " ;
    if ( bad_p ) {
      msg << " ERROR: " << permutationOrd << " >= "
          << m_cell->permutation_count ;
    }
    else {
      msg << permutationOrd ;
    }
    msg << " , " ;
    if ( bad_n ) {
      msg << " ERROR: " << nodeOrd << " >= " << m_cell->node_count ;
    }
    else {
      msg << nodeOrd ;
    }
    msg << " )" ;
    throw std::invalid_argument( msg.str() );
  }
}

// 1D --------------------------------------------------------------------

CellTopologyPrivate::CellTopologyPrivate(
  const std::string & name,
  const unsigned      node_count )
  : m_count_ref(0),
    m_cell(),
    m_name(name),
    m_subcell(1),
    m_node_map()
{
 
  // This 1-subcell is the cell itself, will be assigned to m_cell.subcell[1]
  m_subcell[0].topology = & m_cell ;
  m_subcell[0].node     = index_identity_array();

  m_cell.base = getCellTopologyData< Line<2> >();
  m_cell.name = m_name.c_str();
  m_cell.key  = cellTopologyKey( 1 , 0 , 0 , 2 , node_count );
  m_cell.dimension              = 1 ;
  m_cell.vertex_count           = 2 ;
  m_cell.node_count             = node_count;                                  // PBB 12-03-08
  m_cell.edge_count             = 0 ;
  m_cell.side_count             = 0 ;
  m_cell.subcell_homogeneity[0] = 1 ;
  m_cell.subcell_homogeneity[1] = 0 ;
  m_cell.subcell_homogeneity[2] = 0 ;
  m_cell.subcell_homogeneity[3] = 0 ;
  m_cell.subcell_count[0]       = node_count ;
  m_cell.subcell_count[1]       = 1 ;
  m_cell.subcell_count[2]       = 0 ;
  m_cell.subcell_count[3]       = 0 ;
  m_cell.subcell[0]             = subcell_nodes_array();
  m_cell.subcell[1]             = & m_subcell[0] ;
  m_cell.subcell[2]             = NULL ;
  m_cell.subcell[3]             = NULL ;
  m_cell.side                   = NULL ;
  m_cell.edge                   = NULL ;
}


//2D --------------------------------------------------------------------


CellTopologyPrivate::CellTopologyPrivate(
  const std::string                             & name,
  const unsigned                                  vertex_count ,
  const unsigned                                  node_count,
  const std::vector< const CellTopologyData * > & edges ,
  const std::vector< unsigned >                 & edge_node_map ,
  const CellTopologyData                        * base )
  : m_count_ref(0),
    m_cell(),
    m_name(name),
    m_subcell(),
    m_node_map()
{

  // Compute size of the edge map & check edge homogeneity (suffices to compare nodes per edge)
  const unsigned edge_count = edges.size();
  unsigned       edge_map_size = 0 ;
  unsigned       node_count_edge0 = edges[0] -> node_count;
  bool           edge_homogeneity = true;
      
  for ( unsigned i = 0 ; i < edge_count ; ++i ) {
    edge_map_size += edges[i]->node_count ;
    if(node_count_edge0 != edges[i] -> node_count ) edge_homogeneity = false;
  }

  const bool error_base = base && (
                          base->base         != base ||
                          base->vertex_count != base->node_count ||
                          base->vertex_count != vertex_count ||
                          base->edge_count   != edges.size() );

  const bool error_base_self = ! base && ( vertex_count != node_count );

  const bool error_edge = edge_map_size != edge_node_map.size();

  if ( error_base || error_base_self || error_edge ) {
    // Throw an error
  }

  m_subcell.resize( 1 + edges.size() ),
    
  // This subcell is the cell itself, will be assigned to m_cell.subcell[2]
  m_subcell[ edge_count ].topology = & m_cell ;
  m_subcell[ edge_count ].node     = index_identity_array();

  m_node_map.resize( edge_map_size );

  for ( unsigned i = 0 ; i < edge_map_size ; ++i ) {
    m_node_map[i] = edge_node_map[i];
  }

  edge_map_size = 0 ;
  for ( unsigned i = 0 ; i < edge_count ; ++i ) {
    m_subcell[i].topology = edges[i] ;
    m_subcell[i].node     = & m_node_map[ edge_map_size ] ;
    edge_map_size += edges[i]->node_count ;
  }

  m_cell.base = (base == NULL ? & m_cell : base) ;                            // PBB 12-03-08
  m_cell.name = m_name.c_str();
  m_cell.key  = cellTopologyKey( 2, 0, edge_count, vertex_count, node_count );
  m_cell.dimension              = 2 ;
  m_cell.vertex_count           = vertex_count ;
  m_cell.node_count             = node_count ;                                // PBB 12-03-08
  m_cell.edge_count             = edge_count ;
  m_cell.side_count             = 0 ;
  m_cell.subcell_homogeneity[0] = 1 ;
  m_cell.subcell_homogeneity[1] = edge_homogeneity ;
  m_cell.subcell_homogeneity[2] = 0 ;
  m_cell.subcell_homogeneity[3] = 0 ;
  m_cell.subcell_count[0]       = node_count ;
  m_cell.subcell_count[1]       = edge_count ;
  m_cell.subcell_count[2]       = 1 ;                                           // PBB 12-03-08
  m_cell.subcell_count[3]       = 0 ;
  m_cell.subcell[0]             = subcell_nodes_array();
  m_cell.subcell[1]             = & m_subcell[0] ;
  m_cell.subcell[2]             = & m_subcell[ edge_count ] ;
  m_cell.subcell[3]             = NULL ;
  m_cell.side                   = m_cell.subcell[1] ;
  m_cell.edge                   = m_cell.subcell[1] ;
}


//3D--------------------------------------------------------------------

CellTopologyPrivate::CellTopologyPrivate(const std::string                             & name,
                                         const unsigned                                  vertex_count,
                                         const unsigned                                  node_count,
                                         const std::vector< const CellTopologyData * > & edges ,
                                         const std::vector< unsigned >                 & edge_node_map ,
                                         const std::vector< const CellTopologyData * > & faces ,
                                         const std::vector< unsigned >                 & face_node_map ,
                                         const CellTopologyData                        * base ) 
  : m_count_ref(0),
    m_cell(),
    m_name(name),
    m_subcell(),
    m_node_map()

{
  const unsigned edge_count = edges.size();
  unsigned edge_map_size = 0 ;
  unsigned node_count_edge0 = edges[0] -> node_count;
  bool edge_homogeneity = true;

  // Compute size of the edge map & check edge homogeneity (suffices to compare nodes per edge)
  for ( unsigned i = 0 ; i < edge_count ; ++i ) {
    edge_map_size += edges[i] -> node_count ;
    if(node_count_edge0 != edges[i] -> node_count ) edge_homogeneity = false;
  }
  
  // Compute size of the face map & check face homogeneity (to do)
  const unsigned face_count = faces.size();
  unsigned face_map_size = 0 ;
  for ( unsigned i = 0 ; i < face_count ; ++i ) {
    face_map_size += faces[i] -> node_count ;
  }
  
  // Set error flags for base, edges and faces
  const bool error_base = base && (base->base         != base             ||
                                   base->vertex_count != base->node_count ||
                                   base->vertex_count != vertex_count     ||
                                   base->edge_count   != edges.size()     ||
                                   base->side_count   != faces.size() );
  
  const bool error_base_self = ! base && ( vertex_count != node_count );
  
  const bool error_edge = edge_map_size != edge_node_map.size();
  
  const bool error_face = face_map_size != face_node_map.size();
  
  if ( error_base || error_base_self || error_edge || error_face) {
    // Throw an error
  }
  
  // Flat array for the 1,2,3-subcells of the custom cell.
  m_subcell.resize( 1 + edges.size() + faces.size() ),
    
  // The last subcell is the cell itself & will be assigned to m_cell.subcell[3]
  m_subcell[ edge_count + face_count].topology = & m_cell ;
  m_subcell[ edge_count + face_count].node     = index_identity_array();
  
  // Flat array with edge nodes followed by face nodes (edges and faces can be non-homogeneous)
  m_node_map.resize( edge_map_size + face_map_size);
  
  // Copy edge nodes followed by face nodes
  for ( unsigned i = 0 ; i < edge_map_size ; ++i ) {
    m_node_map[i] = edge_node_map[i];
  }
  for ( unsigned i = 0 ; i < face_map_size ; ++i ) {
    m_node_map[edge_map_size + i] = face_node_map[i];
  }
  
  
  // Copy edge topologies & list of nodes for each edge to m_subcell:
  edge_map_size = 0 ;
  for ( unsigned i = 0 ; i < edge_count ; ++i ) {
    m_subcell[i].topology = edges[i] ;
    m_subcell[i].node     = & m_node_map[ edge_map_size ] ;
    edge_map_size        += edges[i] -> node_count ;
  }
  
  // Copy face topologies & list of nodes for each face to m_subcell:
  face_map_size = 0;
  for ( unsigned i = 0 ; i < face_count ; ++i ) {
    m_subcell[edge_count + i].topology = faces[i] ;
    m_subcell[edge_count + i].node     = & m_node_map[ edge_map_size + face_map_size ] ;
    face_map_size                     += faces[i] -> node_count ;
  }
  
  // Fill CellTopologyData with custom cell data: default base is the custom cell itself
  m_cell.base = (base == NULL ? & m_cell : base) ;                           
  m_cell.name = m_name.c_str();
  m_cell.key  = cellTopologyKey( 3, 
                                 face_count, 
                                 edge_count, 
                                 vertex_count, 
                                 node_count );
  m_cell.dimension              = 3 ;
  m_cell.vertex_count           = vertex_count ;
  m_cell.node_count             = node_count ;                                
  m_cell.edge_count             = edge_count ;
  m_cell.side_count             = face_count ;
  m_cell.subcell_homogeneity[0] = 1 ;
  m_cell.subcell_homogeneity[1] = edge_homogeneity ;
  m_cell.subcell_homogeneity[2] = 0 ;
  m_cell.subcell_homogeneity[3] = 0 ;
  m_cell.subcell_count[0]       = node_count ;
  m_cell.subcell_count[1]       = edge_count ;
  m_cell.subcell_count[2]       = face_count ;                                           
  m_cell.subcell_count[3]       = 1 ;
  m_cell.subcell[0]             = subcell_nodes_array();
  m_cell.subcell[1]             = & m_subcell[0] ;
  m_cell.subcell[2]             = & m_subcell[ edge_count ] ;
  m_cell.subcell[3]             = & m_subcell[ edge_count + face_count] ;
  m_cell.side                   = m_cell.subcell[2] ;
  m_cell.edge                   = m_cell.subcell[1] ;  
}



//----------------------------------------------------------------------


void badCellTopologyKey( const unsigned dimension ,
                         const unsigned face_count ,
                         const unsigned edge_count ,
                         const unsigned vertex_count ,
                         const unsigned node_count )
{
  const unsigned end_dimension    = 1u << 3 ;
  const unsigned end_face_count   = 1u << 6 ;
  const unsigned end_edge_count   = 1u << 6 ;
  const unsigned end_vertex_count = 1u << 6 ;
  const unsigned end_node_count   = 1u << 10 ;

  std::ostringstream msg ;
  msg << "shards::badCellTopologyKey( " ;
  msg << " dimension = " << dimension ;
  if ( dimension >= end_dimension )
    { msg << " > " << end_dimension << " ERROR"; }
  msg << " , face_count = " << face_count ;
  if ( face_count >= end_face_count )
    { msg << " > " << end_face_count << " ERROR"; }
  msg << " , edge_count = " << edge_count ;
  if ( edge_count >= end_edge_count )
    { msg << " > " << end_edge_count << " ERROR"; }
  msg << " , vertex_count = " << vertex_count ;
  if ( vertex_count >= end_vertex_count )
    { msg << " > " << end_vertex_count << " ERROR"; }
  msg << " , node_count = " << node_count ;
  if ( node_count >= end_node_count )
    { msg << " > " << end_node_count << " ERROR"; }
  msg << " )" ;

  throw std::invalid_argument( msg.str() );
}


std::ostream & operator << ( std::ostream & os, const CellTopology & cell) {
  os << *cell.getTopology();
  return os;
}


void getTopologies(std::vector<shards::CellTopology>& topologies,
                   const unsigned cellDim,
                   const ECellType cellType,
                   const ETopologyType topologyType)
{
 
  if ( 4 < cellDim ) {
    std::ostringstream msg ;
    msg << "shards::CellTopology::getTopologies( ERROR: dim = "
      << cellDim << " > 4 )" ;
    throw std::invalid_argument( msg.str() );
  }

  // clear the vector  
  topologies.clear();
  
  // 0-dimensional cells
  if( (cellDim == 0) || (cellDim == 4) ) {
    if( cellType == STANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<Node>() ) ); 
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        // No such cells exist
      }
    }
    if( cellType == NONSTANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        // No such cells exist
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        // No such cells exist
      }
    }
  } // dim 0
  
  
  // 1-dimensional cells
  if((cellDim == 1) || (cellDim == 4)) {
    
    if( cellType == STANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Line<2> >() ) ); 
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Line<3> >() ) );
      }
    }    
    if( cellType == NONSTANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<Particle>() ) );
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        // No such cells exist
      }
    }
  } // dim 1
  
  
  // 2-dimensional cells
  if((cellDim == 2) || (cellDim == 4)) {
    if( cellType == STANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Triangle<3> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Quadrilateral<4> >() ) );
      }
      if (topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Triangle<4> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Triangle<6> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Quadrilateral<8> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Quadrilateral<9> >() ) );
      }      
    }
    if( cellType == NONSTANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellLine<2> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Beam<2> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Pentagon<5> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Hexagon<6> >() ) );
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellLine<3> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Beam<3> >() ) );  
      }      
    }
  } // dim 2
  
  
  if((cellDim == 3) || (cellDim == 4)) {
    if( cellType == STANDARD_CELL || cellType == ALL_CELLS){
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Tetrahedron<4> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Hexahedron<8> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Pyramid<5> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Wedge<6> >() ) );
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Tetrahedron<8> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Tetrahedron<10> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Hexahedron<20> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Hexahedron<27> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Pyramid<13> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Pyramid<14> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Wedge<15> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::Wedge<18> >() ) );
      }      
    }
    if( cellType == NONSTANDARD_CELL || cellType == ALL_CELLS){
      // Predefined Polyhedrons should  go here
      if(topologyType == BASE_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellTriangle<3> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellQuadrilateral<4> >() ) );
      }
      if(topologyType == EXTENDED_TOPOLOGY || topologyType == ALL_TOPOLOGIES) {
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellTriangle<6> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellQuadrilateral<8> >() ) );
        topologies.push_back( CellTopology( shards::getCellTopologyData<shards::ShellQuadrilateral<9> >() ) );
      }      
    }
  } // dim 3    
} // getTopologies


int isPredefinedCell(const CellTopology& cell) {
  
  switch(cell.getKey() ) {
    case Node::key:
    case Particle::key:
    case Line<2>::key:
    case Line<3>::key:
    case ShellLine<2>::key:
    case ShellLine<3>::key:
    case Beam<2>::key:
    case Beam<3>::key:
      
    case Triangle<3>::key:
    case Triangle<4>::key:
    case Triangle<6>::key:
    case ShellTriangle<3>::key:
    case ShellTriangle<6>::key:
      
    case Quadrilateral<4>::key:
    case Quadrilateral<8>::key:
    case Quadrilateral<9>::key:
    case ShellQuadrilateral<4>::key:
    case ShellQuadrilateral<8>::key:
    case ShellQuadrilateral<9>::key:
      
    case Tetrahedron<4>::key:
    case Tetrahedron<8>::key:
    case Tetrahedron<10>::key:
      
    case Hexahedron<8>::key:
    case Hexahedron<20>::key:
    case Hexahedron<27>::key:
      
    case Pyramid<5>::key:
    case Pyramid<13>::key:
    case Pyramid<14>::key:
      
    case Wedge<6>::key:
    case Wedge<15>::key:
    case Wedge<18>::key:
      
    case Pentagon<5>::key:
    case Hexagon<6>::key:
      return 1;
      break;
      
    default:
      return 0;
  }
  
}


} // namespace shards



/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/


#ifndef stk_mesh_Property_hpp
#define stk_mesh_Property_hpp

//----------------------------------------------------------------------

#include <iosfwd>
#include <string>
#include <map>
#include <vector>

#include <stk_mesh/base/Types.hpp>

//----------------------------------------------------------------------

namespace stk {
namespace mesh {

/** \addtogroup stk_mesh_module
 *  \{
 */

/** \brief  Print the text name for a field. */
std::ostream & operator << ( std::ostream & , const PropertyBase & );

/** \brief  Print field and field restrictions on new lines. */
std::ostream & print( std::ostream & ,
                      const char * const , const PropertyBase & );

/** \brief  Internal function to throw a descriptive error message */
void property_data_throw( const PropertyBase & , const Part & );

/** \brief  Query pointer to property data for a given part */
template< typename property_type >
inline
const typename property_type::data_type *
property_data( const property_type & prop , const Part & part )
{
  if ( & prop.mesh_meta_data() != & part.mesh_meta_data() ) {
    property_data_throw( prop , part );
  }
  return prop.data( part.mesh_meta_data_ordinal() );
}

/** \brief  Query pointer to property data for a given part */
template< typename property_type >
inline
typename property_type::data_type *
property_data( property_type & prop , const Part & part )
{
  if ( & prop.mesh_meta_data() != & part.mesh_meta_data() ) {
    property_data_throw( prop , part );
  }
  return prop.data( part.mesh_meta_data_ordinal() );
}

/** \} */
//----------------------------------------------------------------------
//----------------------------------------------------------------------
/** \ingroup stk_mesh_module
 *  \brief  Property base class with an anonymous data type and
 *          anonymous multi-dimension.
 */
template<>
class Property< void > {
public:
  /** \brief  The \ref stk::mesh::MetaData "meta data manager"
   *          that owns this property.
   */
  MetaData & mesh_meta_data() const { return m_meta_data ; }

  /** \brief  Internally generated ordinal of this property that is unique
   *          within the owning \ref stk::mesh::MetaData "meta data manager".
   */
  unsigned mesh_meta_data_ordinal() const { return m_meta_data_ordinal ; }

  /** \brief  Application-defined text name of this property */
  const std::string & name() const { return m_name ; }

  /** \brief  Query if the scalar type is DataType */
  template<class DataType> bool type_is() const
    { return m_type == typeid(DataType); }

  /** \brief  Query number of members, if an array */
  unsigned size() const { return m_size ; }

  /** \brief  Type-checked cast to property with members of the given type */
  template< typename DataType >
  Property< DataType > * property()
    {
      Property< DataType > * p = NULL ;
      if ( m_type == typeid(DataType) ) {
        p = static_cast< Property< DataType > * >( this ); 
      }
      return p ;
    }

  /** \brief  Type-checked cast to property with members of the given type */
  template< typename DataType >
  const Property< DataType > * property() const
    {
      const Property< DataType > * p = NULL ;
      if ( m_type == typeid(DataType) ) {
        p = static_cast< const Property< DataType > * >( this ); 
      }
      return p ;
    }

  //----------------------------------------

#ifndef DOXYGEN_COMPILE

protected:

  Property( MetaData & meta_data ,
            unsigned meta_data_ordinal ,
            const std::string & name ,
            const std::type_info & type ,
            unsigned n )
    : m_name( name ),
      m_meta_data( meta_data ),
      m_meta_data_ordinal( meta_data_ordinal ),
      m_type( type ), m_size( n ) {}

  virtual void add_property( unsigned ) = 0 ;

  virtual ~Property();

private:

  const std::string      m_name ;       ///< Name of the property
  MetaData             & m_meta_data ;  ///< In which this property resides
  const unsigned         m_meta_data_ordinal ;
  const std::type_info & m_type ;       ///< Member data type
  const unsigned         m_size ;       ///< Number of items

  Property();
  Property( const Property & );
  Property & operator = ( const Property & );
 
  friend class MetaData ;
  friend class UnitTestMetaData ;

#endif /* DOXYGEN_COMPILE */
};

//----------------------------------------------------------------------

/** \ingroup stk_mesh_module
 *  \brief  Property with defined data type and multi-dimensions (if any)
 */

template< typename DataType >
class Property : public PropertyBase {
#ifndef DOXYGEN_COMPILE
private:
  friend class MetaData ;

  typedef std::map< unsigned , DataType > map_scalar ;

  map_scalar m_data_scalar ;

protected:

  Property( MetaData & meta_data, unsigned meta_data_ordinal ,
            const std::string & name, unsigned size = 1 )
    : PropertyBase( meta_data, meta_data_ordinal ,
                    name, typeid(DataType), size ) {}

  virtual void add_property( unsigned key ) { m_data_scalar[ key ]; }

  virtual ~Property() {}

#endif /* DOXYGEN_COMPILE */
public:

  /** \brief  Type of the properties' members */
  typedef DataType data_type ;

  /** \brief  Access the properties' members */
  virtual data_type * data( unsigned key )
  {
    const typename map_scalar::iterator i = m_data_scalar.find( key );
    return i != m_data_scalar.end() ? & (*i).second : (data_type*) NULL ;
  }

  /** \brief  Access the properties' members */
  virtual const data_type * data( unsigned key ) const
  {
    const typename map_scalar::const_iterator i = m_data_scalar.find( key );
    return i != m_data_scalar.end() ? & (*i).second : (data_type*) NULL ;
  }

};

#ifndef DOXYGEN_COMPILE

template< typename DataType >
class Property< std::vector< DataType > > : public Property<DataType> {
private:
  friend class MetaData ;

  typedef std::map< unsigned , std::vector< DataType > > map_array ;

  map_array m_data_array ;

  void add_property( unsigned key )
    { m_data_array[ key ].resize( Property<void>::size() ); }

  ~Property() {}
  Property();
  Property( const Property & );
  Property & operator = ( const Property & );

public:

  Property( MetaData & meta_data ,
            unsigned   meta_data_ordinal ,
            const std::string & name ,
            unsigned size )
    : Property<DataType>( meta_data, meta_data_ordinal, name, size ) {}

  typedef DataType data_type ;

  data_type * data( unsigned key )
  {
    const typename map_array::iterator i = m_data_array.find( key );
    return i != m_data_array.end() ? & (*i).second[0] : (data_type*) NULL ;
  }

  const data_type * data( unsigned key ) const
  {
    const typename map_array::const_iterator i = m_data_array.find( key );
    return i != m_data_array.end() ? & (*i).second[0] : (data_type*) NULL ;
  }
};

#endif /* DOXYGEN_COMPILE */

//----------------------------------------------------------------------
//----------------------------------------------------------------------

} // namespace mesh
} // namespace stk

#endif /* stk_mesh_Property_hpp */


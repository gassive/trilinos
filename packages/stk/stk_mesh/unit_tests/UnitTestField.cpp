/*------------------------------------------------------------------------*/
/*                 Copyright 2010 Sandia Corporation.                     */
/*  Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive   */
/*  license for use of this work by or on behalf of the U.S. Government.  */
/*  Export of this program may require a license from the                 */
/*  United States Government.                                             */
/*------------------------------------------------------------------------*/


#include <stk_util/unit_test_support/stk_utest_macros.hpp>

#include <stdexcept>
#include <iostream>
#include <stk_util/parallel/Parallel.hpp>
#include <stk_mesh/base/MetaData.hpp>
#include <stk_mesh/base/BulkData.hpp>

#include <stk_mesh/base/FieldData.hpp>
#include <stk_mesh/fem/EntityRanks.hpp>
#include <stk_mesh/fem/CoordinateSystems.hpp>
#include <stk_mesh/fem/TopologicalMetaData.hpp>

namespace {
void testCartesian();
void testCylindrical();
void testFullTensor();
void testSymmetricTensor();
void testFieldDataArray( stk::ParallelMachine pm );

STKUNIT_UNIT_TEST(UnitTestField, testUnit)
{
#if defined( STK_HAS_MPI )
  stk::ParallelMachine pworld = MPI_COMM_WORLD ;
  stk::ParallelMachine pself  = MPI_COMM_SELF ;
#else
  stk::ParallelMachine pworld = stk::parallel_machine_null();
  stk::ParallelMachine pself  = stk::parallel_machine_null();
#endif
  if ( 0 == stk::parallel_machine_rank( pworld ) ) {
    // Nothing parallel being tested, only run on one process
    testCartesian();
    testCylindrical();
    testFullTensor();
    testSymmetricTensor();
    testFieldDataArray( pself );
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------

SHARDS_ARRAY_DIM_TAG_SIMPLE_DECLARATION( ATAG )
SHARDS_ARRAY_DIM_TAG_SIMPLE_DECLARATION( BTAG )
SHARDS_ARRAY_DIM_TAG_SIMPLE_DECLARATION( CTAG )

template< class FieldType >
void print_bucket_array( const FieldType & f , const stk::mesh::Bucket & k )
{
  typedef stk::mesh::BucketArray< FieldType > ArrayType ;

  try {
    ArrayType a( f , k.begin(), k.end() );
    ArrayType b( f , k );

    std::cout << "  BucketArray[" << f.name() << "](" ;

    if ( a.size() != b.size() ) {
      throw std::runtime_error("UnitTestField FAILED BucketArray dimensions not consistant with Bucket::iterator");
    }

    if ( a.size() ) {
      for ( unsigned i = 0 ; i < ArrayType::Rank ; ++i ) {
        if ( i ) { std::cout << "," ; }
        std::cout << a.dimension(i);
        if (a.dimension(i) != b.dimension(i)) {
          throw std::runtime_error("UnitTestField FAILED BucketArray dimensions not consistant with Bucket::iterator");
        }
      }
    }
    std::cout << ")" << std::endl ;
  }
  catch( const std::exception & ) {
  }
}

void testCartesian()
{
  const stk::mesh::Cartesian&  cartesian_tag = stk::mesh::Cartesian::tag();

  std::string to_str = cartesian_tag.to_string(3, 1);
  std::string expected_str("y");
  STKUNIT_ASSERT_EQUAL( to_str == expected_str, true );

  //should throw if we supply a size < 3:
  STKUNIT_ASSERT_THROW( cartesian_tag.to_string(2, 1), std::runtime_error );

  shards::ArrayDimTag::size_type expected_idx = 1;
  shards::ArrayDimTag::size_type idx = cartesian_tag.to_index(3, "y");

  STKUNIT_ASSERT_EQUAL( idx, expected_idx );

  //should throw if we supply a "z" along with size==2:
  STKUNIT_ASSERT_THROW( cartesian_tag.to_index(2, "z"), std::runtime_error );
}

void testCylindrical()
{
  const stk::mesh::Cylindrical&  cylindrical_tag = stk::mesh::Cylindrical::tag();

  std::string to_str = cylindrical_tag.to_string(3, 1);
  std::string expected_str("a");
  STKUNIT_ASSERT_EQUAL( to_str == expected_str, true );

  //should throw if we supply a size < 3:
  STKUNIT_ASSERT_THROW( cylindrical_tag.to_string(2, 1), std::runtime_error );

  shards::ArrayDimTag::size_type expected_idx = 1;
  shards::ArrayDimTag::size_type idx = cylindrical_tag.to_index(3, "a");

  STKUNIT_ASSERT_EQUAL( idx, expected_idx );

  //should throw if we supply a "z" along with size==2:
  STKUNIT_ASSERT_THROW( cylindrical_tag.to_index(2, "z"), std::runtime_error );
}

void testFullTensor()
{
  const stk::mesh::FullTensor&  fulltensor_tag = stk::mesh::FullTensor::tag();

  std::string to_str = fulltensor_tag.to_string(9, 1);
  std::string expected_str("yx");
  STKUNIT_ASSERT_EQUAL( to_str == expected_str, true );

  //should throw if we supply a size < 9:
  STKUNIT_ASSERT_THROW( fulltensor_tag.to_string(2, 1), std::runtime_error );

  shards::ArrayDimTag::size_type expected_idx = 1;
  shards::ArrayDimTag::size_type idx = fulltensor_tag.to_index(9, "yx");

  STKUNIT_ASSERT_EQUAL( idx, expected_idx );

  //should throw if we supply a "zz" along with size==2:
  STKUNIT_ASSERT_THROW( fulltensor_tag.to_index(2, "zz"), std::runtime_error );
}

void testSymmetricTensor()
{
  const stk::mesh::SymmetricTensor&  symmetrictensor_tag = stk::mesh::SymmetricTensor::tag();

  std::string to_str = symmetrictensor_tag.to_string(9, 1);
  std::string expected_str("yx");
  STKUNIT_ASSERT_EQUAL( to_str == expected_str, true );

  //should throw if we supply a size < 9:
  STKUNIT_ASSERT_THROW( symmetrictensor_tag.to_string(2, 1), std::runtime_error );


  shards::ArrayDimTag::size_type expected_idx = 1;
  shards::ArrayDimTag::size_type idx = symmetrictensor_tag.to_index(6, "yy");

  STKUNIT_ASSERT_EQUAL( idx, expected_idx );

  //should throw if we supply a "xz" along with size==5:
  STKUNIT_ASSERT_THROW( symmetrictensor_tag.to_index(5, "xz"), std::runtime_error );
}

void testFieldDataArray( stk::ParallelMachine pm )
{
  typedef stk::mesh::Field<double>                rank_zero_field ;
  typedef stk::mesh::Field<double,ATAG>           rank_one_field ;
  typedef stk::mesh::Field<double,ATAG,BTAG>      rank_two_field ;
  typedef stk::mesh::Field<double,ATAG,BTAG,CTAG> rank_three_field ;

  std::cout << "UnitTestField BEGIN:" << std::endl ;

  const std::string name0("test_field_0");
  const std::string name1("test_field_1");
  const std::string name2("test_field_2");
  const std::string name3("test_field_3");

  const int spatial_dimension = 3;
  stk::mesh::MetaData meta_data( stk::mesh::TopologicalMetaData::entity_rank_names(spatial_dimension) );
  stk::mesh::BulkData bulk_data( meta_data , pm );
  stk::mesh::TopologicalMetaData top_data( meta_data, spatial_dimension );

  rank_zero_field  & f0 = meta_data.declare_field< rank_zero_field >( name0 );
  rank_one_field   & f1 = meta_data.declare_field< rank_one_field >(  name1 );
  rank_three_field & f3 = meta_data.declare_field< rank_three_field >( name3 );
  rank_two_field   & f2 = meta_data.declare_field< rank_two_field >(  name2 );

  {
    int ok = 0 ;
    try {
      typedef stk::mesh::Field<double,CTAG> error_type ;
      meta_data.declare_field< error_type >( name1 );
    }
    catch( const std::exception & x ) {
      ok = 1 ;
      std::cout << "UnitTestField CORRECTLY caught error: "
                << x.what()
                << std::endl ;
    }
    if ( ! ok ) {
      throw std::runtime_error("UnitTestField FAILED to catch error");
    }
  }

  stk::mesh::Part & p0 = meta_data.declare_part("P0", top_data.node_rank );
  stk::mesh::Part & p1 = meta_data.declare_part("P1", top_data.node_rank );
  stk::mesh::Part & p2 = meta_data.declare_part("P2", top_data.node_rank );
  stk::mesh::Part & p3 = meta_data.declare_part("P3", top_data.node_rank );

  stk::mesh::put_field( f0 , top_data.node_rank , p0 );
  stk::mesh::put_field( f1 , top_data.node_rank , p1 , 10 );
  stk::mesh::put_field( f2 , top_data.node_rank , p2 , 10 , 20 );
  stk::mesh::put_field( f3 , top_data.node_rank , p3 , 10 , 20 , 30 );

  stk::mesh::print( std::cout , "  " , f0 ); std::cout << std::endl ;

  meta_data.commit();

  bulk_data.modification_begin();

  for ( unsigned i = 1 ; i < 11 ; ++i ) {
    bulk_data.declare_entity( top_data.node_rank , i ,
                              std::vector< stk::mesh::Part * >( 1 , & p0 ) );
  }

  for ( unsigned i = 11 ; i < 21 ; ++i ) {
    bulk_data.declare_entity( top_data.node_rank , i ,
                              std::vector< stk::mesh::Part * >( 1 , & p1 ) );
  }

  for ( unsigned i = 21 ; i < 31 ; ++i ) {
    bulk_data.declare_entity( top_data.node_rank , i ,
                              std::vector< stk::mesh::Part * >( 1 , & p2 ) );
  }

  for ( unsigned i = 31 ; i < 41 ; ++i ) {
    bulk_data.declare_entity( top_data.node_rank , i ,
                              std::vector< stk::mesh::Part * >( 1 , & p3 ) );
  }

  const std::vector< stk::mesh::Bucket *> & node_buckets =
    bulk_data.buckets( top_data.node_rank );

  for ( std::vector< stk::mesh::Bucket *>::const_iterator
        ik = node_buckets.begin() ; ik != node_buckets.end() ; ++ik ) {
    stk::mesh::Bucket & k = **ik ;

    std::vector< stk::mesh::Part * > parts ;
    k.supersets( parts );

    std::cout << "Bucket:" ;
    for ( std::vector< stk::mesh::Part * >::iterator
          ip = parts.begin() ; ip != parts.end() ; ++ip ) {
      std::cout << " " << (*ip)->name();
    }
    std::cout << std::endl ;

    print_bucket_array( f0 , k );
    print_bucket_array( f1 , k );
    print_bucket_array( f2 , k );
    print_bucket_array( f3 , k );
  }

  std::cout << "UnitTestField END" << std::endl ;
}

SHARDS_ARRAY_DIM_TAG_SIMPLE_IMPLEMENTATION( ATAG )
SHARDS_ARRAY_DIM_TAG_SIMPLE_IMPLEMENTATION( BTAG )
SHARDS_ARRAY_DIM_TAG_SIMPLE_IMPLEMENTATION( CTAG )

}//namespace <anonymous>


/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2004 Sandia Corporation and Argonne National
    Laboratory.  Under the terms of Contract DE-AC04-94AL85000 
    with Sandia Corporation, the U.S. Government retains certain 
    rights in this software.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License 
    (lgpl.txt) along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    diachin2@llnl.gov, djmelan@sandia.gov, mbrewer@sandia.gov, 
    pknupp@sandia.gov, tleurent@mcs.anl.gov, tmunson@mcs.anl.gov      
   
  ***************************************************************** */

/*! \file TerminationCriterionTest.cpp
    \author kraftche@cae.wisc.edu

Tests for the TerminationCriterion class.. 

*/


#include "Mesquite.hpp"
#include "MsqError.hpp"
#include "Vector3D.hpp"
#include "OFEvaluator.hpp"
#include "ObjectiveFunction.hpp"
#include "PatchData.hpp"
#include "TerminationCriterion.hpp"

#include "UnitUtil.hpp"
#include "PatchDataInstances.hpp"

#include <iostream>

using namespace Mesquite;


class DummyOF : public ObjectiveFunction
{
public:
  double mValue; //!< Objectve fuction value returned
  Vector3D mGrad; //!< Gradient values for all vertices
  bool mValid;

  DummyOF( double of_value = 0.0,
           Vector3D grad_values = Vector3D(0,0,0) )
    : mValue(0.0), mGrad(grad_values), mValid(true) {}

  bool initialize_block_coordinate_descent( Mesh* mesh,
                                            MeshDomain* domain,
                                            const Settings* settings,
                                            PatchSet* user_set,
                                            MsqError& err );
    
  void initialize_queue( Mesh* , MeshDomain* , 
                         const Settings* ,
                         MsqError&  ) {}

  bool evaluate( EvalType type,
                 PatchData& pd,
                 double& value_out,
                 bool free,
                 MsqError& err );

  bool evaluate_with_gradient( EvalType type,
                               PatchData& pd,
                               double& value_out,
                               std::vector<Vector3D>& grad_out,
                               MsqError& err );
                               
  ObjectiveFunction* clone() const { return new DummyOF(*this); }
  void clear() {}
  int min_patch_layers() const { return 1; }
};

class TerminationCriterionTest : public CppUnit::TestFixture
{
private:
  CPPUNIT_TEST_SUITE(TerminationCriterionTest);

  CPPUNIT_TEST (test_number_of_iterates_inner);
  CPPUNIT_TEST (test_number_of_iterates_outer);
  CPPUNIT_TEST (test_cpu_time_inner);
  CPPUNIT_TEST (test_cpu_time_outer);

  CPPUNIT_TEST (test_absolute_vertex_movement);
  CPPUNIT_TEST (test_relative_vertex_movement);

  CPPUNIT_TEST (test_gradient_L2_norm_absolute);
  CPPUNIT_TEST (test_gradient_Linf_norm_absolute);
  CPPUNIT_TEST (test_gradient_L2_norm_relative);
  CPPUNIT_TEST (test_gradient_Linf_norm_relative);

  CPPUNIT_TEST (test_quality_improvement_absolute);
  CPPUNIT_TEST (test_quality_improvement_relative);
  CPPUNIT_TEST (test_successive_improvements_absolute);
  CPPUNIT_TEST (test_successive_improvements_relative);

  CPPUNIT_TEST (test_vertex_bound);
  CPPUNIT_TEST (test_untangled_mesh);

  CPPUNIT_TEST_SUITE_END();
  
  DummyOF objFunc;
  OFEvaluator ofEval;
  
  void test_gradient_common( bool absolute, bool L2 );
  void test_quality_common( bool absolute, bool successive );
  void test_vertex_movement_common( bool absolute );
  void test_cpu_time_common( bool inner );
  
public:

  TerminationCriterionTest()
    : ofEval( &objFunc, true ) {}
  
    //NUMBER OF ITERATES
  void test_number_of_iterates_inner();
  void test_number_of_iterates_outer();
  
    //CPU TIME
  void test_cpu_time_inner()
    { test_cpu_time_common( true ); }
  void test_cpu_time_outer()
    { test_cpu_time_common( false ); }
  
    // VERTEX MOVEMENT
  void test_absolute_vertex_movement()
    { test_vertex_movement_common( true ); }
  void test_relative_vertex_movement()
    { test_vertex_movement_common( false ); }
  
    //GRADIENT NORM ABSOLUTE
  void test_gradient_L2_norm_absolute()
    {  test_gradient_common( true, true ); }
  void test_gradient_Linf_norm_absolute()
    {  test_gradient_common( true, false ); }
  
    //GRADIENT NORM RELATIVE
  void test_gradient_L2_norm_relative()
    {  test_gradient_common( false, true ); }
  void test_gradient_Linf_norm_relative()
    {  test_gradient_common( false, false ); }
  
    //QUALITY IMPROVEMENT ABSOLUTE
  void test_quality_improvement_absolute()
    { test_quality_common( true, false ); }

    //QUALITY IMPROVEMENT RELATIVE
  void test_quality_improvement_relative()
    { test_quality_common( false, false ); }

    //SUCCESSIVE IMPROVEMENTS ABSOLUTE
  void test_successive_improvements_absolute()
    { test_quality_common( true, true ); }
  
    //SUCCESSIVE IMPROVEMENTS RELATIVE
  void test_successive_improvements_relative()
    { test_quality_common( false, true ); }
  
    //VERTEX BOUND
  void test_vertex_bound();
  
  void test_untangled_mesh();
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TerminationCriterionTest, "TerminationCriterionTest");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(TerminationCriterionTest, "Unit");

  
    //NUMBER OF ITERATES
void TerminationCriterionTest::test_number_of_iterates_inner()
{
  MsqPrintError err(std::cout);
  PatchData pd;
  const int LIMIT = 2;

  TerminationCriterion tc;
  tc.add_iteration_limit(LIMIT);
  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  for (int i = 0; i < LIMIT; ++i) {
    CPPUNIT_ASSERT(!tc.terminate());
    CPPUNIT_ASSERT_EQUAL( i, tc.get_iteration_count() );
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
    tc.accumulate_inner( pd, 0, 0, err );
    ASSERT_NO_ERROR(err);
  }
  CPPUNIT_ASSERT_EQUAL( 2, tc.get_iteration_count() );
  CPPUNIT_ASSERT(tc.terminate());
}

void TerminationCriterionTest::test_number_of_iterates_outer()
{
  MsqPrintError err(std::cout);
  PatchData pd;
  const int LIMIT = 2;

  TerminationCriterion tc;
  tc.add_iteration_limit(LIMIT);
  tc.reset_outer( 0, 0, ofEval, 0, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  for (int i = 0; i < LIMIT; ++i) {
    CPPUNIT_ASSERT(!tc.terminate());
    CPPUNIT_ASSERT_EQUAL( i, tc.get_iteration_count() );
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
    tc.accumulate_outer( 0, 0, ofEval, 0, err );
    ASSERT_NO_ERROR(err);
  }
  CPPUNIT_ASSERT_EQUAL( 2, tc.get_iteration_count() );
  CPPUNIT_ASSERT(tc.terminate());
}

  
    //CPU TIME
void TerminationCriterionTest::test_cpu_time_common( bool inner )
{
  MsqPrintError err(std::cout);
  PatchData pd;
  double LIMIT = 1.5;

  TerminationCriterion tc;
  tc.add_cpu_time(LIMIT);
  if (inner)
    tc.reset_inner( pd, ofEval, err );
  else
    tc.reset_outer( 0, 0, ofEval, 0, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  
  Timer timer;
  while (timer.since_birth() < 0.5*LIMIT) {
    CPPUNIT_ASSERT(!tc.terminate());
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
    if (inner)
      tc.accumulate_inner( pd, 0, 0, err );
    else
      tc.accumulate_outer( 0, 0, ofEval, 0, err );
    ASSERT_NO_ERROR(err);
  }
  
  while (timer.since_birth() < 1.1*LIMIT);

  tc.accumulate_patch( pd, err );
  ASSERT_NO_ERROR(err);
  if (inner)
    tc.accumulate_inner( pd, 0, 0, err );
  else
    tc.accumulate_outer( 0, 0, ofEval, 0, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(tc.terminate());
}


void TerminationCriterionTest::test_vertex_movement_common( bool absolute )
{
  MsqPrintError err(std::cout);
  PatchData pd;
  create_twelve_hex_patch( pd, err );
  ASSERT_NO_ERROR(err);
 
  const double LIMIT = 1e-4;
  TerminationCriterion tc;
  if (absolute)
    tc.add_absolute_vertex_movement( LIMIT );
  else
    tc.add_relative_vertex_movement( LIMIT );

  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(!tc.terminate());

  const double FIRST_STEP=10.0;
    // move a vertex by 10 units and check that it did not meet criterion
  pd.move_vertex( Vector3D(FIRST_STEP,0,0), 0, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_inner( pd, 0.0, 0, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(!tc.terminate());
  
  double test_limit = LIMIT;
  if (!absolute)
    test_limit *= FIRST_STEP;
    
  int idx = 0;
  for (double step = FIRST_STEP; step > test_limit; step *= 0.09) {
    idx = (idx + 1) % pd.num_free_vertices();
    pd.move_vertex( Vector3D(step,0,0), idx, err );
    ASSERT_NO_ERROR(err);
    
    tc.accumulate_inner( pd, 0.0, 0, err );
    ASSERT_NO_ERROR(err);
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
    CPPUNIT_ASSERT(!tc.terminate());
  }
  
  idx = (idx + 1) % pd.num_free_vertices();
  pd.move_vertex( Vector3D(0.5*test_limit,0,0), idx, err );
  ASSERT_NO_ERROR(err);

  tc.accumulate_inner( pd, 0.0, 0, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(tc.terminate());
}

static double lenfunc( const Vector3D* vect, int len )
  { return Mesquite::length( vect, len ); }
static double maxfunc( const Vector3D* vect, int len )
  { return Mesquite::Linf( vect, len ); }

void TerminationCriterionTest::test_gradient_common( bool absolute, bool L2 )
{
  MsqPrintError err(std::cout);
  PatchData pd;
  create_twelve_hex_patch( pd, err );
  ASSERT_NO_ERROR(err);
  
  const double LIMIT = 1e-4;
  TerminationCriterion tc;
  if (absolute) {
    if (L2)
      tc.add_absolute_gradient_L2_norm( LIMIT );
    else
      tc.add_absolute_gradient_inf_norm( LIMIT );
  }
  else {
    if (L2)
      tc.add_relative_gradient_L2_norm( LIMIT );
    else
      tc.add_relative_gradient_inf_norm( LIMIT );
  }
  
  double (*func_ptr)(const Vector3D*, int) = L2 ? &lenfunc : &maxfunc;

  double junk, value = 1; 
  objFunc.mGrad = Vector3D(value,value,value);
  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  
  std::vector<Vector3D> grad;
  ofEval.evaluate( pd, junk, grad, err );
  ASSERT_NO_ERROR(err);
  
  double limit = LIMIT;
  if (!absolute) 
    limit *= func_ptr(&grad[0],pd.num_free_vertices());

  while (func_ptr(&grad[0],pd.num_free_vertices()) > limit) {
    CPPUNIT_ASSERT(!tc.terminate());

    value *= 0.1;
    objFunc.mGrad = Vector3D(value,value,value);
    ofEval.evaluate( pd, junk, grad, err );
    ASSERT_NO_ERROR(err);
 
    tc.accumulate_inner( pd, 0.0, &grad[0], err );
    ASSERT_NO_ERROR(err);
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
  }
  
  CPPUNIT_ASSERT(tc.terminate());
}

static bool limit_absolute_quality( double ,
                                    double ,
                                    double curr_value,
                                    double epsilon )
{ return curr_value <= epsilon; }
static bool limit_relative_quality( double init_value,
                                    double ,
                                    double curr_value,
                                    double epsilon )
{ return curr_value <= epsilon*init_value; }
static bool limit_absolute_sucessive( double ,
                                      double prev_value,
                                      double curr_value,
                                      double epsilon )
{ return (prev_value - curr_value) <= epsilon; }
static bool limit_relative_sucessive( double init_value,
                                      double prev_value,
                                      double curr_value,
                                      double epsilon )
{ return (prev_value - curr_value) <= epsilon*(init_value-curr_value); }

void TerminationCriterionTest::test_quality_common( bool absolute, bool successive )
{
  MsqPrintError err(std::cout);
  PatchData pd;
  ASSERT_NO_ERROR(err);
  
  const double LIMIT = 1e-4;
  TerminationCriterion tc;
  bool (*func_ptr)(double, double, double, double);
  if (absolute) {
    if (successive) {
      tc.add_absolute_successive_improvement( LIMIT );
      func_ptr = &limit_absolute_sucessive;
    }
    else {
      tc.add_absolute_quality_improvement( LIMIT );
      func_ptr = &limit_absolute_quality;
    }
  }
  else {
    if (successive) {
      tc.add_relative_successive_improvement( LIMIT );
      func_ptr = &limit_relative_sucessive;
    }
    else {
      tc.add_relative_quality_improvement( LIMIT );
      func_ptr = &limit_relative_quality;
    }
  }

  const double INIT_VALUE = 10.0;
  objFunc.mValue = INIT_VALUE;
  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  double prev = HUGE_VAL;

  while (!func_ptr(INIT_VALUE,prev,objFunc.mValue,LIMIT)) {
    CPPUNIT_ASSERT(!tc.terminate());

    prev = objFunc.mValue;
    objFunc.mValue *= 0.1;
    tc.accumulate_inner( pd, objFunc.mValue, 0, err );
    ASSERT_NO_ERROR(err);
    tc.accumulate_patch( pd, err );
    ASSERT_NO_ERROR(err);
  }
  
  CPPUNIT_ASSERT(tc.terminate());
}

//VERTEX BOUND
void TerminationCriterionTest::test_vertex_bound()
{
  MsqPrintError err(std::cout);
  PatchData pd;
  create_twelve_hex_patch( pd, err );
  ASSERT_NO_ERROR(err);
  
    // get bounding dimension for patch
  double maxcoord = 0.0;
  for (size_t i = 0; i < pd.num_nodes(); ++i) 
    for (int d = 0; d < 3; ++d)
      if (fabs(pd.vertex_by_index(i)[d]) > maxcoord)
        maxcoord = fabs(pd.vertex_by_index(i)[d]);
    // add a little bit for rounding error
  maxcoord += 1e-5;
  
  TerminationCriterion tc;
  tc.add_bounded_vertex_movement( maxcoord );
  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(!tc.terminate());
  
  int idx = pd.num_free_vertices() - 1;
  Vector3D pos = pd.vertex_by_index(idx);
  pos[0] = 2*maxcoord;
  pd.set_vertex_coordinates( pos, idx, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_inner( pd, 0.0, 0, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(tc.terminate());
}



//UNTANGLED
void TerminationCriterionTest::test_untangled_mesh()
{
  MsqPrintError err(std::cout);
  PatchData pd;
  create_twelve_hex_patch( pd, err );
  ASSERT_NO_ERROR(err);
  
    // get two opposite vertices in first hexahedral element
  int vtx1 = pd.element_by_index(0).get_vertex_index_array()[0];
  int vtx2 = pd.element_by_index(0).get_vertex_index_array()[7];
  Vector3D saved_coords = pd.vertex_by_index(vtx2);
  Vector3D opposite_coords = pd.vertex_by_index(vtx1);

    // invert the element
  pd.move_vertex( 2*(opposite_coords-saved_coords), vtx2, err );
  ASSERT_NO_ERROR(err);
  int inverted, samples;
  pd.element_by_index(0).check_element_orientation( pd, inverted, samples, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(inverted > 0);
  
    // check initial termination criterion
  TerminationCriterion tc;
  tc.add_untangled_mesh();
  tc.reset_inner( pd, ofEval, err );
  ASSERT_NO_ERROR(err);
  tc.reset_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(!tc.terminate());
  
    // fix the element
  pd.set_vertex_coordinates( saved_coords, vtx2, err );
  ASSERT_NO_ERROR(err);
  pd.element_by_index(0).check_element_orientation( pd, inverted, samples, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT_EQUAL(0,inverted);
  
    // check that TC recognized untangled mesh
  tc.accumulate_inner( pd, 0.0, 0, err );
  ASSERT_NO_ERROR(err);
  tc.accumulate_patch( pd, err );
  ASSERT_NO_ERROR(err);
  CPPUNIT_ASSERT(tc.terminate());
}



bool DummyOF::initialize_block_coordinate_descent( Mesh*,
                                                   MeshDomain*,
                                                   const Settings*,
                                                   PatchSet*,
                                                   MsqError& err )
{
  MSQ_SETERR(err)(MsqError::NOT_IMPLEMENTED);
  return false;
}

bool DummyOF::evaluate( EvalType, PatchData&, double& value, bool, MsqError& )
{
  value = mValue;
  return mValid;
}

bool DummyOF::evaluate_with_gradient( EvalType,
                                      PatchData& pd,
                                      double& value_out,
                                      std::vector<Vector3D>& grad_out,
                                      MsqError& )
{
  value_out = mValue;
  grad_out.clear();
  grad_out.resize( pd.num_free_vertices(), mGrad );
  return mValid;
}

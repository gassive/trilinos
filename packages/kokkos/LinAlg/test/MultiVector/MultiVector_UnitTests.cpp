// ************************************************************************
// 
//          Kokkos: A Fast Kernel Package
//              Copyright (2004) Sandia Corporation
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
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ************************************************************************
//@HEADER

#include <Teuchos_UnitTestHarness.hpp>
#include <Teuchos_TimeMonitor.hpp>
#include <Teuchos_Time.hpp>
#include <Teuchos_TypeNameTraits.hpp>

#include "Kokkos_ConfigDefs.hpp"
#include "Kokkos_DefaultNode.hpp"
#include "Kokkos_MultiVector.hpp"
#include "Kokkos_Version.hpp"

namespace {

  using Kokkos::DefaultNode;
  using Kokkos::MultiVector;
  using Teuchos::ArrayRCP;
  using Teuchos::RCP;

  size_t N = 1000;

  typedef Kokkos::DefaultNode::DefaultNodeType Node;

  TEUCHOS_STATIC_SETUP()
  {
    Teuchos::CommandLineProcessor &clp = Teuchos::UnitTestRepository::getCLP();
    clp.addOutputSetupOptions(true);
    int n = N;
    clp.setOption("test-size",&n,"Vector length for tests.");
    N = n;
  }

  //
  // UNIT TESTS
  // 

  // check that default constructor zeros out, for both V and MV
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( MultiVector, DefaultConstructor, Scalar, Ordinal )
  {
    RCP<Node> node = Kokkos::DefaultNode::getDefaultNode();
    MultiVector<Scalar,Node> A(node);
    TEST_EQUALITY_CONST(A.getNumRows(), 0);
    TEST_EQUALITY_CONST(A.getNumCols(), 0);
    TEST_EQUALITY_CONST(A.getStride(), 0);
  }

  // check copy constructor
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( MultiVector, CopyConstructor, Scalar, Ordinal )
  {
    RCP<Node> node = Kokkos::DefaultNode::getDefaultNode();
    MultiVector<Scalar,Node> A(node);
    ArrayRCP<Scalar> buf = A.getNode()->template allocBuffer<Scalar>(2*N);
    A.initializeValues(N,2,buf,N);
    {
      MultiVector<Scalar,Node> Acopy(A);
      TEST_EQUALITY_CONST(Acopy.getNumRows(), N);
      TEST_EQUALITY_CONST(Acopy.getNumCols(), 2);
      TEST_EQUALITY_CONST(Acopy.getStride(), N);
      TEST_EQUALITY(Acopy.getValues(0), buf);
      TEST_INEQUALITY(Acopy.getValues(1), buf);
    }
  }

  // check that non-default constructor honors given parameters
  TEUCHOS_UNIT_TEST_TEMPLATE_2_DECL( MultiVector, InitializeAndAccess, Scalar, Ordinal )
  {
    RCP<Node> node = Kokkos::DefaultNode::getDefaultNode();
    MultiVector<Scalar,Node> A(node);
    ArrayRCP<Scalar> buf = A.getNode()->template allocBuffer<Scalar>(2*N);
    A.initializeValues(N,2,buf,N);
    TEST_EQUALITY_CONST(A.getNumRows(), N);
    TEST_EQUALITY_CONST(A.getNumCols(), 2);
    TEST_EQUALITY_CONST(A.getStride(), N);
    TEST_EQUALITY(A.getValues(0), buf);
    TEST_INEQUALITY(A.getValues(1), buf);
    buf = Teuchos::null;
  }


#define UNIT_TEST_GROUP_ORDINAL_SCALAR( ORDINAL, SCALAR ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( MultiVector, DefaultConstructor, SCALAR, ORDINAL ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( MultiVector, CopyConstructor   , SCALAR, ORDINAL ) \
  TEUCHOS_UNIT_TEST_TEMPLATE_2_INSTANT( MultiVector, InitializeAndAccess, SCALAR, ORDINAL )

#define UNIT_TEST_GROUP_ORDINAL( ORDINAL ) \
  UNIT_TEST_GROUP_ORDINAL_SCALAR(ORDINAL, int) \
  UNIT_TEST_GROUP_ORDINAL_SCALAR(ORDINAL, float)
  UNIT_TEST_GROUP_ORDINAL(int)
    typedef short int ShortInt; UNIT_TEST_GROUP_ORDINAL(ShortInt)

}

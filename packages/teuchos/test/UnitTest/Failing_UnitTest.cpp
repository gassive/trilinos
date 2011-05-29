/*
// @HEADER
// ***********************************************************************
//
//                    Teuchos: Common Tools Package
//                 Copyright (2004) Sandia Corporation
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
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER
*/

#include "Teuchos_UnitTestHarness.hpp"


namespace {


TEUCHOS_UNIT_TEST( Int,  BadAssignment )
{
  int i1 = 4;
  int i2 = i1 + 1;
  TEST_EQUALITY( i2, i1 );
}


TEUCHOS_UNIT_TEST( VectorInt, OutOfRangeAt )
{
  const size_t n = 1;
  std::vector<int> v(n);
  const int i = v.at(n); // Should throw std::out_of_range!
  TEST_EQUALITY_CONST( i, 10 ); // Will never be executed!
}


} // namespace

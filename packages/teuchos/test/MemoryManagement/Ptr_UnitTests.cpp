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
#include "Teuchos_Ptr.hpp"
#include "Teuchos_getConst.hpp"
#include "TestClasses.hpp"


namespace {


using Teuchos::null;
using Teuchos::Ptr;
using Teuchos::RCP;
using Teuchos::rcp;
using Teuchos::ptrFromRef;
using Teuchos::rcpFromPtr;
using Teuchos::NullReferenceError;
using Teuchos::DanglingReferenceError;
using Teuchos::RCP_STRONG;
using Teuchos::RCP_WEAK;
using Teuchos::RCP_STRENGTH_INVALID;


TEUCHOS_UNIT_TEST( Ptr, nonnull )
{
  ECHO(A a);
  ECHO(Ptr<A> a_ptr = ptrFromRef(a));
  TEST_EQUALITY_CONST(is_null(a_ptr), false);
  TEST_EQUALITY_CONST(nonnull(a_ptr), true);
  ECHO(a_ptr = null);
  TEST_EQUALITY_CONST(is_null(a_ptr), true);
  TEST_EQUALITY_CONST(nonnull(a_ptr), false);
}


TEUCHOS_UNIT_TEST( Ptr, getConst )
{
  RCP<A> a_rcp(new A);
  Ptr<A> a_ptr = a_rcp.ptr();
  Ptr<const A> ca_ptr = a_ptr.getConst();
  TEST_EQUALITY(a_ptr.getRawPtr(), ca_ptr.getRawPtr());
}


TEUCHOS_UNIT_TEST( Ptr, rcpFromPtr_weakRef )
{
  ECHO(RCP<A> a_rcp = rcp(new A));
  ECHO(Ptr<A> a_ptr = a_rcp.ptr());
  ECHO(RCP<A> a_rcp2 = rcpFromPtr(a_ptr));
  TEST_EQUALITY(a_rcp2.getRawPtr(), a_rcp.getRawPtr());
#ifdef TEUCHOS_DEBUG
  TEST_ASSERT(a_rcp2.shares_resource(a_rcp));
#else
  // In an optimized build, the object a_rcp2 has its own RCPNode object that
  // is unrelated to the orgininal a_rcp object.  This cuts down on overhead.
#endif
  ECHO(a_rcp = null);
#ifdef TEUCHOS_DEBUG
  TEST_THROW(a_ptr.getRawPtr(), DanglingReferenceError);
  TEST_THROW(a_rcp2.getRawPtr(), DanglingReferenceError);
#endif
  
}


TEUCHOS_UNIT_TEST( Ptr, rcpFromPtr_rawRef )
{
  ECHO(A a);
  ECHO(Ptr<A> a_ptr = ptrFromRef(a));
  ECHO(RCP<A> a_rcp2 = rcpFromPtr(a_ptr));
  TEST_EQUALITY(a_rcp2.getRawPtr(), &a);
}


TEUCHOS_UNIT_TEST( Ptr, rcpFromPtr_null )
{
  ECHO(Ptr<A> a_ptr);
  ECHO(RCP<A> a_rcp2 = rcpFromPtr(a_ptr));
  TEST_EQUALITY(a_rcp2, null);
}


} // namespace

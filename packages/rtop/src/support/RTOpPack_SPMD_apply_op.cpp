// @HEADER
// ***********************************************************************
// 
// RTOp: Interfaces and Support Software for Vector Reduction Transformation
//       Operations
//                Copyright (2006) Sandia Corporation
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
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#include "RTOpPack_SPMD_apply_op_decl.hpp"


#ifdef RTOPPACK_ENABLE_SHOW_DUMP
bool RTOpPack::show_spmd_apply_op_dump = false;
#endif // RTOPPACK_ENABLE_SHOW_DUMP


#ifdef HAVE_RTOP_EXPLICIT_INSTANTIATION


#include "RTOpPack_SPMD_apply_op_def.hpp"
#include "Teuchos_ExplicitInstantiationHelpers.hpp"


namespace RTOpPack {


TEUCHOS_MACRO_TEMPLATE_INSTANT_SCALAR_TYPES(
  RTOPPACK_SPMD_APPLY_OP_INSTANT_SCALAR)


} // namespace RTOpPack


#endif // HAVE_TEUCHOS_EXCPLICIT_INSTANTIATION

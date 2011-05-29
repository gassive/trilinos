/*
// @HEADER
// ***********************************************************************
// 
//    OptiPack: Collection of simple Thyra-based Optimization ANAs
//                 Copyright (2009) Sandia Corporation
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
*/

#ifndef OPTIPACK_TYPES_HPP
#define OPTIPACK_TYPES_HPP


#include "OptiPack_ConfigDefs.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_Ptr.hpp"
#include "Teuchos_Array.hpp"
#include "Teuchos_ArrayView.hpp"
#include "Teuchos_ScalarTraits.hpp"


namespace Teuchos {

/** \brief . */
class ParameterList;

} // namespace Teuchos


namespace OptiPack {


/** \brief . */
using Teuchos::RCP;
/** \brief . */
using Teuchos::Ptr;
/** \brief . */
using Teuchos::Array;
/** \brief . */
using Teuchos::ArrayView;
/** \brief . */
using Teuchos::ScalarTraits;
/** \brief . */
using Teuchos::ParameterList;


} // namespace OptiPack


namespace Thyra {

/** \brief . */
template<class Scalar> class ModelEvaluator;



} // namespace Thyra


#endif // OPTIPACK_TYPES_HPP

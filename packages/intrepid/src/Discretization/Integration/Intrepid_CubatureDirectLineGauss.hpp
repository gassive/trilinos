// @HEADER
// ************************************************************************
//
//                           Intrepid Package
//                 Copyright (2007) Sandia Corporation
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
// Questions? Contact Pavel Bochev (pbboche@sandia.gov) or
//                    Denis Ridzal (dridzal@sandia.gov).
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_CubatureDirectLineGauss.hpp
    \brief  Header file for the Intrepid::CubatureDirectLineGauss class.
    \author Created by P. Bochev and D. Ridzal.
*/

#ifndef INTREPID_CUBATURE_DIRECT_LINE_GAUSS_HPP
#define INTREPID_CUBATURE_DIRECT_LINE_GAUSS_HPP

#include "Intrepid_ConfigDefs.hpp"
#include "Intrepid_CubatureDirect.hpp"
#include "Teuchos_TestForException.hpp"

/** \def INTREPID_CUBATURE_LINE_GAUSS_MAX
  \brief The maximum degree of the polynomial that can be integrated exactly by
         a direct line rule of the Gauss(-Legendre) type.
*/
// srkenno@sandia.gov 6/21/10:
// see below comment for the enum
#define INTREPID_CUBATURE_LINE_GAUSS_MAX 61


namespace Intrepid {

/** \class Intrepid::CubatureDirectLineGauss
    \brief Defines Gauss integration rules on a line.
*/
template<class Scalar, class ArrayPoint = FieldContainer<Scalar>, class ArrayWeight = ArrayPoint>
class CubatureDirectLineGauss : public Intrepid::CubatureDirect<Scalar,ArrayPoint,ArrayWeight> {
  public:

  // srkenno@sandia.gov 6/21/10:
  // This indirection is to workaround a compiler bug on the sun platform, 5.7 toolset, SunOS 10.
  enum {INTREPID_CUBATURE_LINE_GAUSS_MAX_ENUM = INTREPID_CUBATURE_LINE_GAUSS_MAX};

  private:

  /** \brief Complete set of data defining line Gauss(-Legendre) rules.
  */
  static const CubatureTemplate cubature_data_[INTREPID_CUBATURE_LINE_GAUSS_MAX_ENUM+1];

  /** \brief Names of templates for frequently used direct cubature rules.
  */
  static const char *cubature_name_;


  public:

  ~CubatureDirectLineGauss() {}

  /** \brief Constructor.

      \param degree           [in]     - The degree of polynomials that are integrated
                                         exactly by this cubature rule. Default: 0.
  */
  CubatureDirectLineGauss(const int degree = 0);

  /** \brief Returns cubature name.
  */
  const char* getName() const;

  /** \brief Exposes cubature data.
  */
  const CubatureTemplate * exposeCubatureData() const;

  /** \brief Returns maximum cubature accuracy.
  */
  int getMaxAccuracy() const;

  /** \brief Exposes cubature data, accessible without construction.
  */
  static const CubatureTemplate (& exposeCubatureDataStatic())[INTREPID_CUBATURE_LINE_GAUSS_MAX_ENUM+1];


}; // end class CubatureDirect 

template<class Scalar, class ArrayPoint, class ArrayWeight>
inline const CubatureTemplate (& CubatureDirectLineGauss<Scalar,ArrayPoint,ArrayWeight>::exposeCubatureDataStatic())[INTREPID_CUBATURE_LINE_GAUSS_MAX_ENUM+1] {
  return cubature_data_;
}

} // end namespace Intrepid


// include templated definitions
#include <Intrepid_CubatureDirectLineGaussDef.hpp>

#endif

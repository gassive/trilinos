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
//                    Denis Ridzal (dridzal@sandia.gov) or
//                    Robert Kirby (robert.c.kirby@ttu.edu)
//
// ************************************************************************
// @HEADER


/** \file
\brief  Unit test of OrthogonalBases class
\author Created by P. Bochev and D. Ridzal.
*/

#include "Intrepid_FieldContainer.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Intrepid_OrthogonalBases.hpp"
#include <Intrepid_CubatureDirectTetDefault.hpp>
#include <iostream>
using namespace Intrepid;

/** \brief outdated tests for orthogonal bases
    \param argc [in] - number of command-line arguments
    \param argv [in] - command-line arguments
 */
int main(int argc, char *argv[]) {

  Teuchos::GlobalMPISession mpiSession(&argc, &argv);
  
  // This little trick lets us print to std::cout only if a (dummy) command-line argument is provided.
  int iprint     = argc - 1;
  
  Teuchos::RCP<std::ostream> outStream;
  Teuchos::oblackholestream bhs; // outputs nothing
  
  if (iprint > 0)
    outStream = Teuchos::rcp(&std::cout, false);
  else
    outStream = Teuchos::rcp(&bhs, false);
  
  // Save the format state of the original std::cout.
  Teuchos::oblackholestream oldFormatState;
  oldFormatState.copyfmt(std::cout);
  
  *outStream \
    << "===============================================================================\n" \
    << "|                                                                             |\n" \
    << "|                           Unit Test OrthogonalBases                         |\n" \
    << "|                                                                             |\n" \
    << "|     1) Tests orthogonality of tetrahedral orthogonal basis                  |\n" \
    << "|                                                                             |\n" \
    << "|  Questions? Contact  Pavel Bochev (pbboche@sandia.gov) or                   |\n" \
    << "|                      Denis Ridzal (dridzal@sandia.gov) or                   |\n" \
    << "|                      Robert Kirby (robert.c.kirby@ttu.edu)                  |\n" \
    << "|                                                                             |\n" \
    << "|  Intrepid's website: http://trilinos.sandia.gov/packages/intrepid           |\n" \
    << "|  Trilinos website:   http://trilinos.sandia.gov                             |\n" \
    << "|                                                                             |\n" \
    << "===============================================================================\n";
  
  int errorFlag  = 0;
  
  // First, get a reference quadrature rule

  CubatureDirectTetDefault<double,FieldContainer<double> > myCub(20);
  FieldContainer<double> cubPts( myCub.getNumPoints() , 3 );
  FieldContainer<double> cubWts( myCub.getNumPoints() );

  myCub.getCubature( cubPts , cubWts );
  
  // Tabulate the basis functions at the cubature points
  const int deg = 10;
  const int polydim = (deg+1)*(deg+2)*(deg+3)/6;
  FieldContainer<double> basisAtCubPts( polydim , myCub.getNumPoints() );
  OrthogonalBases::tabulateTetrahedron<double,FieldContainer<double>,FieldContainer<double> >( cubPts , deg , basisAtCubPts );

  // Now let's compute the mass matrix
  for (int i=0;i<polydim;i++) {
    for (int j=0;j<polydim;j++) {
      double cur = 0;
      for (int k=0;k<myCub.getNumPoints();k++) {
        cur += cubWts(k) * basisAtCubPts( i , k ) * basisAtCubPts( j , k );
      }
      if (i != j && fabs( cur ) > 20.0 * INTREPID_TOL) {
        std::cout << INTREPID_TOL << std::endl;
        std::cout << i << " " << j << " " << cur << std::endl;
        errorFlag++;
      }
      else if (i == j && fabs( cur ) < 20.0 * INTREPID_TOL ) {
        std::cout << i << " " << j << " " << cur << std::endl;
        errorFlag++;
      }

    }
  }

  
  if (errorFlag != 0)
    std::cout << "End Result: TEST FAILED\n";
  else
    std::cout << "End Result: TEST PASSED\n";
  
  // reset format state of std::cout
  std::cout.copyfmt(oldFormatState);
  
  return errorFlag;
}

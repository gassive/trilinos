/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2009
 Sandia National Laboratories.  Developed at the
    University of Wisconsin--Madison under SNL contract number
    624796.  The U.S. Government and the University of Wisconsin
    retain certain rights to this software.

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

    (2009) kraftche@cae.wisc.edu    

  ***************************************************************** */


/** \file Target3DSizeBarrier.cpp
 *  \brief 
 *  \author Jason Kraftcheck 
 */

#include "Mesquite.hpp"
#include "Target3DSizeBarrier.hpp"
#include "MsqMatrix.hpp"
#include "TMPDerivs.hpp"

namespace MESQUITE_NS {

Target3DSizeBarrier::~Target3DSizeBarrier() {}

std::string Target3DSizeBarrier::get_name() const
  { return "SizeBarrier"; }

bool Target3DSizeBarrier::evaluate( const MsqMatrix<3,3>& A, 
                                    const MsqMatrix<3,3>& W, 
                                    double& result, 
                                    MsqError& )
{
  MsqMatrix<3,3> T = A * inverse(W);
  double d = det(T);
  if (invalid_determinant(d)) {
    result = 0.0;
    return false;
  }
  result = d + 1.0/d - 2.0;
  return true;  
}

bool Target3DSizeBarrier::evaluate_with_grad( const MsqMatrix<3,3>& A,
                                              const MsqMatrix<3,3>& W,
                                              double& result,
                                              MsqMatrix<3,3>& deriv_wrt_A,
                                              MsqError& err )
{
  const MsqMatrix<3,3> Winv = inverse(W);
  const MsqMatrix<3,3> T = A * Winv;
  double d = det(T);
  if (invalid_determinant(d)) {
    result = 0.0;
    return false;
  }
  result = d + 1.0/d - 2.0;
  deriv_wrt_A = (1 - 1/(d*d)) * transpose_adj(T) * transpose(Winv);
  return true;  
}
  
bool Target3DSizeBarrier::evaluate_with_hess( const MsqMatrix<3,3>& A,
                                              const MsqMatrix<3,3>& W,
                                              double& result,
                                              MsqMatrix<3,3>& deriv_wrt_A,
                                              MsqMatrix<3,3> second_wrt_A[6],
                                              MsqError& err )
{
  const MsqMatrix<3,3> Winv = inverse(W);
  const MsqMatrix<3,3> T = A * Winv;
  double d = det(T);
  if (invalid_determinant(d)) {
    result = 0.0;
    return false;
  }
  result = d + 1.0/d - 2.0;
  MsqMatrix<3,3> adjt = transpose_adj(T);
  const double f = 1 - 1/(d*d);
  deriv_wrt_A = f * adjt * transpose(Winv);
  
  set_scaled_outer_product( second_wrt_A, 2/(d*d*d), adjt );
  pluseq_scaled_2nd_deriv_of_det( second_wrt_A, f, T );
  second_deriv_wrt_product_factor( second_wrt_A, Winv );
  return true;  
}

} // namespace Mesquite

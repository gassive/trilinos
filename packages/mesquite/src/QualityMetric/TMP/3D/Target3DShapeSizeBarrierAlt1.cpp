/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2009 Sandia National Laboratories.  Developed at the
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


/** \file Target3DShapeSizeBarrierAlt1.cpp
 *  \brief 
 *  \author Jason Kraftcheck 
 */

#include "Mesquite.hpp"
#include "Target3DShapeSizeBarrierAlt1.hpp"
#include "TMPDerivs.hpp"

namespace MESQUITE_NS {

std::string Target3DShapeSizeBarrierAlt1::get_name() const
  { return "ShapeSizeBarrier1"; }

bool Target3DShapeSizeBarrierAlt1::evaluate( const MsqMatrix<3,3>& A, 
                                             const MsqMatrix<3,3>& W, 
                                             double& result, 
                                             MsqError&  )
{
  const MsqMatrix<3,3> T = A * inverse(W);
  const double tau = det(T);
  if (invalid_determinant(tau)) { // barrier
    result = 0.0;
    return false;
  }
  
  const double f = sqr_Frobenius(T);
  const double g = sqr_Frobenius(adj(T));
  result = (f + g)/(6 * tau) - 1;
  return true;
}

bool Target3DShapeSizeBarrierAlt1::evaluate_with_grad( const MsqMatrix<3,3>& A,
                                                       const MsqMatrix<3,3>& W,
                                                       double& result,
                                                       MsqMatrix<3,3>& deriv_wrt_A,
                                                       MsqError& err )
{
  const MsqMatrix<3,3> Winv = inverse(W);
  const MsqMatrix<3,3> T = A * Winv;
  const double tau = det(T);
  if (invalid_determinant(tau)) { // barrier
    result = 0.0;
    return false;
  }
  
  const double f = sqr_Frobenius(T);
  const double g = sqr_Frobenius(adj(T));
  result = (f + g)/(6 * tau);
  
  deriv_wrt_A = -transpose(T) * T;
  deriv_wrt_A(0,0) += 1+f;
  deriv_wrt_A(1,1) += 1+f;
  deriv_wrt_A(2,2) += 1+f;
  deriv_wrt_A = T * deriv_wrt_A;
  deriv_wrt_A -= 3*result * transpose_adj(T);
  deriv_wrt_A *= 1.0/(3*tau);
  deriv_wrt_A = deriv_wrt_A * transpose(Winv);
  
  result -= 1.0;
  return true;
}


bool Target3DShapeSizeBarrierAlt1::evaluate_with_hess( const MsqMatrix<3,3>& A,
                                                       const MsqMatrix<3,3>& W,
                                                       double& result,
                                                       MsqMatrix<3,3>& wrt_A,
                                                       MsqMatrix<3,3> second[6],
                                                       MsqError& err )
{
  const MsqMatrix<3,3> Winv = inverse(W);
  const MsqMatrix<3,3> T = A * Winv;
  const double tau = det(T);
  if (invalid_determinant(tau)) { // barrier
    result = 0.0;
    return false;
  }
  
  const double f = sqr_Frobenius(T);
  const double g = sqr_Frobenius(adj(T));
  result = (f + g)/(6 * tau);
  
  MsqMatrix<3,3> dtau = transpose_adj(T);
  MsqMatrix<3,3> dg = -transpose(T) * T;
  dg(0,0) += f;
  dg(1,1) += f;
  dg(2,2) += f;
  dg = T * dg;
  dg *= 2;
  
  wrt_A = T;
  wrt_A += 0.5*dg;
  wrt_A *= 1.0/3.0;
  wrt_A -= result * dtau;
  wrt_A *= 1.0/tau;
  wrt_A = wrt_A * transpose(Winv);
  
  set_scaled_2nd_deriv_norm_sqr_adj( second, 1.0/6.0, T );
  pluseq_scaled_I( second, 1.0/3.0 );
  pluseq_scaled_sum_outer_product( second, -1./3./tau, T, dtau );
  pluseq_scaled_sum_outer_product( second, -1./6./tau, dg, dtau );
  pluseq_scaled_outer_product( second, 2*result/tau, dtau );
  pluseq_scaled_2nd_deriv_of_det( second, -result, T );
  hess_scale( second, 1.0/tau );
  second_deriv_wrt_product_factor( second, Winv );
  
  result -= 1.0;
  return true;
}

} // namespace Mesquite

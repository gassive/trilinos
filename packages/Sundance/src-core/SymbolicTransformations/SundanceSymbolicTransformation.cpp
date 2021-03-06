/* @HEADER@ */
// ************************************************************************
// 
//                              Sundance
//                 Copyright (2005) Sandia Corporation
// 
// Copyright (year first published) Sandia Corporation.  Under the terms 
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government 
// retains certain rights in this software.
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
// Questions? Contact Kevin Long (krlong@sandia.gov), 
// Sandia National Laboratories, Livermore, California, USA
// 
// ************************************************************************
/* @HEADER@ */

#include "SundanceExpr.hpp"
#include "SundanceScalarExpr.hpp"
#include "SundanceSymbolicTransformation.hpp"


using namespace Sundance;
using namespace Sundance;

using namespace Teuchos;
using namespace Sundance;

SymbolicTransformation::SymbolicTransformation()
{}

RCP<ScalarExpr> SymbolicTransformation::chooseSign(int sign, 
                                                           const RCP<ScalarExpr>& expr) 
{
  /* return expr if sign == 1, -expr if sign == -1. No other
   * cases should happen. */
  switch(sign)
    {
    case 1:
      return expr;
    case -1:
      {
        Expr e = -Expr::handle(expr);
        RCP<ScalarExpr> rtn = rcp_dynamic_cast<ScalarExpr>(e.ptr());
        TEST_FOR_EXCEPTION(rtn.get() == NULL, InternalError,
                           "Non-scalar expr "
                           << e.toString() 
                           << " detected in SymbolicTransformation::chooseSign");
        return rtn;
      }
    default:
      TEST_FOR_EXCEPTION(true, InternalError, 
                         "sign != +/- 1 in Expr::transformSign()");
    }
  return expr;
}

Expr SymbolicTransformation::chooseSign(int sign, 
                                        const Expr& expr) 
{
  /* return expr if sign == 1, -expr if sign == -1. No other
   * cases should happen. */
  switch(sign)
    {
    case 1:
      return expr;
    case -1:
      return -expr;
    default:
      TEST_FOR_EXCEPTION(true, InternalError, 
                         "sign != +/- 1 in Expr::transformSign()");
    }
  return expr;
}

RCP<ScalarExpr> SymbolicTransformation::getScalar(const Expr& expr)
{
  RCP<ScalarExpr> s = rcp_dynamic_cast<ScalarExpr>(expr.ptr());

  TEST_FOR_EXCEPTION(s.get()==NULL, InternalError,
                     "non-scalar detected in SymbolicTransformation::getScalar");

  return s;
}

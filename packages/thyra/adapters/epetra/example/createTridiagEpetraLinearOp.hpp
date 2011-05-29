// @HEADER
// ***********************************************************************
// 
//               Epetra: Linear Algebra Services Package 
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

#ifndef THYRA_CREATE_TRIDIAG_EPETRA_LINEAR_OP_HPP
#define THYRA_CREATE_TRIDIAG_EPETRA_LINEAR_OP_HPP

#include "Thyra_OperatorVectorTypes.hpp"
#ifdef HAVE_MPI
#include "mpi.h"
#endif

class Epetra_Operator;

/** \brief \brief This function generates a tridiagonal linear operator using Epetra.
 *
 * Specifically, this function returns a smart pointer to the matrix:
\f[

A=
\left[\begin{array}{rrrrrrrrrr}
2 a    & -1 \\
-1     &  2 a    & -1 \\
       & \ddots  & \ddots  & \ddots \\
       &         & -1      & 2 a     & -1 \\
       &         &         &  -1     & 2 a
\end{array}\right]
\f]
 *
 * where <tt>diagScale</tt> is \f$a\f$ and <tt>globalDim</tt> is the
 * glboal dimension of the matrix.
 */
Teuchos::RCP<Epetra_Operator>
createTridiagEpetraLinearOp(
  const int      globalDim
#ifdef HAVE_MPI
  ,MPI_Comm      mpiComm
#endif
  ,const double  diagScale
  ,const bool    verbose
  ,std::ostream  &out
  );

#endif // THYRA_CREATE_TRIDIAG_EPETRA_LINEAR_OP_HPP

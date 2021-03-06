// $Id$ 
// $Source$ 

//@HEADER
// ************************************************************************
// 
//            NOX: An Object-Oriented Nonlinear Solver Package
//                 Copyright (2002) Sandia Corporation
// 
//            LOCA: Library of Continuation Algorithms Package
//                 Copyright (2005) Sandia Corporation
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
// 
// Questions? Contact Roger Pawlowski (rppawlo@sandia.gov) or 
// Eric Phipps (etphipp@sandia.gov), Sandia National Laboratories.
// ************************************************************************
//  CVS Information
//  $Source$
//  $Author$
//  $Date$
//  $Revision$
// ************************************************************************
//@HEADER

#ifndef LOCA_BORDEREDSOLVER_UPPERTRIANGULARBLOCKELIMINATION_H
#define LOCA_BORDEREDSOLVER_UPPERTRIANGULARBLOCKELIMINATION_H

#include "LOCA_BorderedSolver_AbstractStrategy.H"  // base class

// forward declarations
namespace LOCA {
  class GlobalData;
  namespace Parameter {
    class SublistParser;
  }
}

namespace LOCA {

  namespace BorderedSolver {
    
    //! Block elimination strategy for solving a block upper-triangular system
    /*! 
     * This class solves the extended system of equations
     * \f[
     *     \begin{bmatrix}
     *        op(J) &   A    \\
     *        0     & op(C)
     *     \end{bmatrix}
     *     \begin{bmatrix}
     *        X \\
     *        Y 
     *     \end{bmatrix} = 
     *     \begin{bmatrix}
     *        F \\
     *        G
     *     \end{bmatrix}
     * \f]
     * via block elimination:
     * \f[
     *     \begin{aligned}
     *        Y &= op(C)^{-1}G \\
     *        X &= op(J)^{-1}(F - A Y)
     *     \end{aligned}
     * \f]
     * where \f$op\f$ represents either the identity operation or the
     * transpose.  \f$C\f$ must be nonzero, while \f$A\f$, \f$F\f$ or \f$G\f$ 
     * may be zero.  The solve
     * for the non-transposed system is implemented by the solve() method,
     * while the solve for the transposed system is implemented by the
     * solveTranspose() method.
    */
    class UpperTriangularBlockElimination {

    public:

      //! Constructor. 
      /*!
       * \param global_data [in] Global data object
       */
      UpperTriangularBlockElimination(
	 const Teuchos::RCP<LOCA::GlobalData>& global_data);

      //! Destructor
      virtual ~UpperTriangularBlockElimination();

      /*! 
       * \brief Solves the extended system as described above.
       */
      /*!
       * Either A, F, or G may be zero by passing NULL.  
       */
      NOX::Abstract::Group::ReturnType  
      solve(Teuchos::ParameterList& params,
	    const LOCA::BorderedSolver::AbstractOperator& op,
	    const NOX::Abstract::MultiVector* A,
	    const NOX::Abstract::MultiVector::DenseMatrix& C,
	    const NOX::Abstract::MultiVector* F,
	    const NOX::Abstract::MultiVector::DenseMatrix* G,
	    NOX::Abstract::MultiVector& X,
	    NOX::Abstract::MultiVector::DenseMatrix& Y) const;

      /*! 
       * \brief Solves the extended system using the tranpose of J and C
       * as described above.
       */
      /*!
       * Either A, F, or G may be zero by passing NULL.  
       */
      NOX::Abstract::Group::ReturnType  
      solveTranspose(Teuchos::ParameterList& params,
		     const LOCA::BorderedSolver::AbstractOperator& op,
		     const NOX::Abstract::MultiVector* A,
		     const NOX::Abstract::MultiVector::DenseMatrix& C,
		     const NOX::Abstract::MultiVector* F,
		     const NOX::Abstract::MultiVector::DenseMatrix* G,
		     NOX::Abstract::MultiVector& X,
		     NOX::Abstract::MultiVector::DenseMatrix& Y) const;
      
    private:

      //! Private to prohibit copying
      UpperTriangularBlockElimination(const UpperTriangularBlockElimination&);

      //! Private to prohibit copying
      UpperTriangularBlockElimination& 
      operator = (const UpperTriangularBlockElimination&);

    protected:

      //! Global data object
      Teuchos::RCP<LOCA::GlobalData> globalData;

    }; // class UpperTriangularBlockElimination
  } // namespace BorderedSolver
} // namespace LOCA

#endif

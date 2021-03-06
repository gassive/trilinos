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

#ifndef LOCA_TURNINGPOINT_MINIMALLYAUGMENTED_ABSTRACTGROUP_H
#define LOCA_TURNINGPOINT_MINIMALLYAUGMENTED_ABSTRACTGROUP_H

#include "LOCA_TurningPoint_MooreSpence_AbstractGroup.H"    // base class

namespace LOCA { 
  
  namespace TurningPoint { 

    /*! 
     * \brief Groups and vectors for locating turning point bifurcations
     * using the minimally augmented turning point formulation.
     */
    namespace MinimallyAugmented {

      /*! 
       * \brief Interface to underlying groups for turning point calculations
       * using the minimally augmented formulation.
       */
      /*!
       * This abstract class provides the required interface for underlying
       * groups to locate turning points using the minimally augmented turning 
       * point formulation (see 
       * LOCA::TurningPoint::MinimallyAugmented::ExtendedGroup for a 
       * description of the governing equations).  
       *
       * This class is derived from the 
       * LOCA::TurningPoint::MooreSpence::AbstractGroup 
       * and declares several pure virtual methods compute various 
       * derivatives of \f$w^TJn\f$ for a given \f$w\f$ and \f$n\f$.  Default 
       * implementations for the derivatives using finite differencing are 
       * implemented in the 
       * LOCA::TurningPoint::MinimallyAugmented::FiniteDifferenceGroup.
       */
      class AbstractGroup : 
	public virtual LOCA::TurningPoint::MooreSpence::AbstractGroup {

      public:
  
	//! Default constructor.
	AbstractGroup() {}

	//! Destructor
	virtual ~AbstractGroup() {}

	/*! 
	 * @name Pure virtual methods 
	 * These methods must be defined by any concrete implementation
	 */
	//@{

	/*! 
	 * \brief Computes the derivative \f$\partial w^TJn/\partial p\f$ 
	 */
	virtual NOX::Abstract::Group::ReturnType 
	computeDwtJnDp(const vector<int>& paramIDs, 
		       const NOX::Abstract::Vector& w,
		       const NOX::Abstract::Vector& nullVector,
		       NOX::Abstract::MultiVector::DenseMatrix& result,
		       bool isValid) = 0;

	/*! 
	 * \brief Computes the derivative \f$\partial w^TJ/\partial p\f$ 
	 */
	virtual NOX::Abstract::Group::ReturnType 
	computeDwtJDp(const vector<int>& paramIDs, 
		      const NOX::Abstract::Vector& w,
		      NOX::Abstract::MultiVector& result,
		      bool isValid) = 0;

	/*! 
	 * \brief Computes the derivative 
	 * \f$\frac{\partial w^TJn}{\partial x}\f$. 
	 */
	virtual NOX::Abstract::Group::ReturnType 
	computeDwtJnDx(const NOX::Abstract::Vector& w,
		       const NOX::Abstract::Vector& nullVector,
		       NOX::Abstract::Vector& result) = 0;

	//@}

	//! Bring NOX::Abstract::Group::operator=() into scope
	using NOX::Abstract::Group::operator=;

      }; // class AbstractGroup

    } // namespace MinimallyAugmented

  } // namespace TurningPoint

} // namespace LOCA

#endif

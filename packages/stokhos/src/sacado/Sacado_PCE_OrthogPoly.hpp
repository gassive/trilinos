// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
//                 Copyright (2006) Sandia Corporation
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
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef SACADO_PCE_ORTHOGPOLY_HPP
#define SACADO_PCE_ORTHOGPOLY_HPP

#include "Stokhos_ConfigDefs.h"

#ifdef HAVE_STOKHOS_SACADO

#include "Teuchos_RCP.hpp"

#include "Sacado_Handle.hpp"

#include "Stokhos_OrthogPolyExpansion.hpp"
#include "Stokhos_OrthogPolyApprox.hpp"

#include <cmath>
#include <algorithm>	// for std::min and std::max
#include <ostream>	// for std::ostream

namespace Sacado {

  //! Namespace for polynomial chaos expansion classes
  namespace PCE {

    //! Generalized polynomial chaos expansion class
    /*!
     * Uses a handle and a "copy-on-write" strategy for efficient copying, but
     * no expression templating.
     */
    template <typename T> 
    class OrthogPoly {
    public:

      //! Turn OrthogPoly into a meta-function class usable with mpl::apply
      template <typename U> 
      struct apply {
	typedef OrthogPoly<U> type;
      };

      //! Typename of values
      typedef T value_type;

      //! Typename of ordinals
      typedef int ordinal_type;

      //! Basis type
      typedef Stokhos::OrthogPolyBasis<ordinal_type,T> basis_type;

      //! Expansion type
      typedef Stokhos::OrthogPolyExpansion<ordinal_type,T> expansion_type;

      //! Default constructor
      /*!
       * Sets size to 1 and first coefficient to 0 (represents a constant).
       */
      OrthogPoly();

      //! Constructor with supplied value \c x
      /*!
       * Sets size to 1 and first coefficient to x (represents a constant).
       */
      OrthogPoly(const value_type& x);

      //! Constructor with expansion \c expansion (General case)
      /*!
       * Creates array of correct size and initializes coeffiencts to 0.
       */
      OrthogPoly(const Teuchos::RCP<expansion_type>& expansion);

      //! Constructor with expansion \c expansion and specified size \c sz
      /*!
       * Creates array of size \c sz and initializes coeffiencts to 0.
       */
      OrthogPoly(const Teuchos::RCP<expansion_type>& expansion,
		 ordinal_type sz);

      //! Copy constructor
      OrthogPoly(const OrthogPoly& x);

      //! Destructor
      ~OrthogPoly();

      //! Reset expansion
      /*!
       * May change size of array.  Coefficients are preserved.  
       */
      void reset(const Teuchos::RCP<expansion_type>& expansion);

      //! Reset expansion and size
      /*!
       * Coefficients are preserved.  
       */
      void reset(const Teuchos::RCP<expansion_type>& expansion,
		 ordinal_type sz);

      //! Prepare polynomial for writing 
      /*!
       * This method prepares the polynomial for writing through coeff() and 
       * fastAccessCoeff() member functions.  It ensures the handle for the
       * %Hermite coefficients is not shared among any other %Hermite polynomial
       * objects.  If the handle is not shared it does nothing, so there
       * is no cost in calling this method in this case.  If the handle is 
       * shared and this method is not called, any changes to the coefficients
       * by coeff() or fastAccessCoeff() may change other polynomial objects.
       */
      void copyForWrite() { th.makeOwnCopy(); }

      //! Evaluate polynomial approximation at a point
      value_type evaluate(const Teuchos::Array<value_type>& point) const;

      //! Evaluate polynomial approximation at a point with given basis values
      value_type evaluate(const Teuchos::Array<value_type>& point,
                          const Teuchos::Array<value_type>& bvals) const;

      //! Compute mean of expansion
      value_type mean() const {return th->mean(); }

      //! Compute standard deviation of expansion
      value_type standard_deviation() const { return th->standard_deviation(); }

      //! Print approximation in basis
      std::ostream& print(std::ostream& os) const { return th->print(os); }

      /*!
       * @name Assignment operators
       */
      //@{

      //! Assignment operator with constant right-hand-side
      OrthogPoly<T>& operator=(const value_type& val);

      //! Assignment with OrthogPoly right-hand-side
      OrthogPoly<T>& operator=(const OrthogPoly<T>& x);

      //@}

      /*!
       * Accessor methods
       */
      //@{

      //! Get basis
      Teuchos::RCP<basis_type> basis() const { return th->basis(); }

      //! Get expansion
      Teuchos::RCP<expansion_type> expansion() const { return expansion_; }

      //@}

      /*!
       * @name Value accessor methods
       */
      //@{

      //! Returns value
      const value_type& val() const { return (*th)[0]; }

      //! Returns value
      value_type& val() { return (*th)[0]; }

      //@}

      /*!
       * @name Coefficient accessor methods
       */
      //@{

      //! Returns size of polynomial
      ordinal_type size() const { return th->size();}

      //! Returns true if polynomial has size >= sz
      bool hasFastAccess(ordinal_type sz) const { return th->size()>=sz;}

      //! Returns Hermite coefficient array
      const value_type* coeff() const { return th->coeff();}

      //! Returns Hermite coefficient array
      value_type* coeff() { return th->coeff();}

      //! Returns degree \c i term with bounds checking
      value_type coeff(ordinal_type i) const { 
	value_type tmp= i<th->size() ? (*th)[i]:value_type(0.); return tmp;}
    
      //! Returns degree \c i term without bounds checking
      value_type& fastAccessCoeff(ordinal_type i) { return (*th)[i];}

      //! Returns degree \c i term without bounds checking
      value_type fastAccessCoeff(ordinal_type i) const { return (*th)[i];}

      //! Get coefficient term for given dimension and order
      value_type& term(ordinal_type dimension, ordinal_type order) {
	return th->term(dimension, order); }

      //! Get coefficient term for given dimension and order
      const value_type& term(ordinal_type dimension, ordinal_type order) const {
	return th->term(dimension, order); }

      //! Get orders for a given term
      Teuchos::Array<ordinal_type> order(ordinal_type term) const {
	return th->order(term); }
    
      //@}

      /*!
       * @name Unary operators
       */
      //@{

      //! Unary-plus operator
      OrthogPoly<T> operator + () const;

      //! Unary-minus operator
      OrthogPoly<T> operator - () const;

      //! Addition-assignment operator with constant right-hand-side
      OrthogPoly<T>& operator += (const value_type& x);

      //! Subtraction-assignment operator with constant right-hand-side
      OrthogPoly<T>& operator -= (const value_type& x);

      //! Multiplication-assignment operator with constant right-hand-side
      OrthogPoly<T>& operator *= (const value_type& x);

      //! Division-assignment operator with constant right-hand-side
      OrthogPoly<T>& operator /= (const value_type& x);

      //! Addition-assignment operator with Hermite right-hand-side
      OrthogPoly<T>& operator += (const OrthogPoly<T>& x);

      //! Subtraction-assignment operator with Hermite right-hand-side
      OrthogPoly<T>& operator -= (const OrthogPoly<T>& x);
  
      //! Multiplication-assignment operator with Hermite right-hand-side
      OrthogPoly<T>& operator *= (const OrthogPoly<T>& x);

      //! Division-assignment operator with Hermite right-hand-side
      OrthogPoly<T>& operator /= (const OrthogPoly<T>& x);

      //@}

      //! Get underlying Stokhos::OrthogPolyApprox
      const Stokhos::OrthogPolyApprox<int,value_type>& getOrthogPolyApprox() const 
      { return *th; }

      //! Get underlying Stokhos::OrthogPolyApprox
      Stokhos::OrthogPolyApprox<int,value_type>& getOrthogPolyApprox() { 
	return *th; }

    protected:

      //! Expansion class
      Teuchos::RCP<expansion_type> expansion_;

      Sacado::Handle< Stokhos::OrthogPolyApprox<int,value_type> > th;

    }; // class Hermite

    // Operations
    template <typename T> OrthogPoly<T> 
    operator+(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator+(const typename OrthogPoly<T>::value_type& a, 
	      const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator+(const OrthogPoly<T>& a, 
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    operator-(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator-(const typename OrthogPoly<T>::value_type& a, 
	      const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator-(const OrthogPoly<T>& a, 
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    operator*(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator*(const typename OrthogPoly<T>::value_type& a, 
	      const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator*(const OrthogPoly<T>& a, 
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    operator/(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator/(const typename OrthogPoly<T>::value_type& a, 
	      const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    operator/(const OrthogPoly<T>& a, 
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    exp(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    log(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    log10(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    sqrt(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    pow(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    pow(const T& a, 
	const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    pow(const OrthogPoly<T>& a, 
	const T& b);

    template <typename T> OrthogPoly<T> 
    cos(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    sin(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    tan(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    cosh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T>
    sinh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    tanh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    acos(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    asin(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    atan(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    atan2(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    atan2(const typename OrthogPoly<T>::value_type& a, 
	  const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    atan2(const OrthogPoly<T>& a, 
	  const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    acosh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    asinh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    atanh(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    abs(const OrthogPoly<T>& a);
    
    template <typename T> OrthogPoly<T> 
    fabs(const OrthogPoly<T>& a);

    template <typename T> OrthogPoly<T> 
    max(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    max(const typename OrthogPoly<T>::value_type& a, 
	const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    max(const OrthogPoly<T>& a, 
	const typename OrthogPoly<T>::value_type& b);

    template <typename T> OrthogPoly<T> 
    min(const OrthogPoly<T>& a, const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    min(const typename OrthogPoly<T>::value_type& a, 
	const OrthogPoly<T>& b);

    template <typename T> OrthogPoly<T> 
    min(const OrthogPoly<T>& a, 
	const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator==(const OrthogPoly<T>& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator==(const typename OrthogPoly<T>::value_type& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator==(const OrthogPoly<T>& a,
	       const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator!=(const OrthogPoly<T>& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator!=(const typename OrthogPoly<T>::value_type& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator!=(const OrthogPoly<T>& a,
	       const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator<=(const OrthogPoly<T>& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator<=(const typename OrthogPoly<T>::value_type& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator<=(const OrthogPoly<T>& a,
	       const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator>=(const OrthogPoly<T>& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator>=(const typename OrthogPoly<T>::value_type& a,
	       const OrthogPoly<T>& b);

    template <typename T> bool 
    operator>=(const OrthogPoly<T>& a,
	       const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator<(const OrthogPoly<T>& a,
	      const OrthogPoly<T>& b);

    template <typename T> bool 
    operator<(const typename OrthogPoly<T>::value_type& a,
	      const OrthogPoly<T>& b);

    template <typename T> bool 
    operator<(const OrthogPoly<T>& a,
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> bool 
    operator>(const OrthogPoly<T>& a,
	      const OrthogPoly<T>& b);

    template <typename T> bool 
    operator>(const typename OrthogPoly<T>::value_type& a,
	      const OrthogPoly<T>& b);

    template <typename T> bool 
    operator>(const OrthogPoly<T>& a,
	      const typename OrthogPoly<T>::value_type& b);

    template <typename T> std::ostream& 
    operator << (std::ostream& os, const OrthogPoly<T>& a);

  } // namespace PCE

} // namespace Sacado

#include "Sacado_PCE_OrthogPolyTraits.hpp"
#include "Sacado_PCE_OrthogPolyImp.hpp"

#endif // HAVE_STOKHOS_SACADO

#endif // SACADO_PCE_ORTHOGPOLY_HPP

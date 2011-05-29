// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Stokhos Package
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
// Questions? Contact Eric T. Phipps (etphipp@sandia.gov).
// 
// ***********************************************************************
// @HEADER

#ifndef STOKHOS_ALGEBRAICORTHOGPOLYEXPANSION_HPP
#define STOKHOS_ALGEBRAICORTHOGPOLYEXPANSION_HPP

#include "Stokhos_OrthogPolyExpansion.hpp"

#include "Teuchos_RCP.hpp"

namespace Stokhos {

  //! Orthogonal polynomial expansions limited to algebraic operations
  /*!
   * Implements +, -, *, / (with constant denominator), and other operations
   * with constant arguments.  Useful for problems that only involve
   * algebraic operations, or constant non-algebraic operations in that it
   * doesn't require creation of additional data structures (like sparse
   * quadrature grids) that may take significant computational time.
   */
  template <typename ordinal_type, typename value_type> 
  class AlgebraicOrthogPolyExpansion : 
    public OrthogPolyExpansion<ordinal_type, value_type> {
  public:

    //! Constructor
    AlgebraicOrthogPolyExpansion(
      const Teuchos::RCP<const OrthogPolyBasis<ordinal_type, value_type> >& basis,
      const Teuchos::RCP<const Stokhos::Sparse3Tensor<ordinal_type, value_type> >& Cijk);

    //! Destructor
    virtual ~AlgebraicOrthogPolyExpansion() {}

    //! Get expansion size
    ordinal_type size() const { return sz; }

    //! Get basis
    Teuchos::RCP< const OrthogPolyBasis<ordinal_type, value_type> > 
    getBasis() const {return basis; }
 
    // Operations
    void unaryMinus(OrthogPolyApprox<ordinal_type, value_type>& c, 
                    const OrthogPolyApprox<ordinal_type, value_type>& a);

    void plusEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
		   const value_type& x);
    void minusEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
		    const value_type& x);
    void timesEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
		    const value_type& x);
    void divideEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
		     const value_type& x);

    void plusEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
                   const OrthogPolyApprox<ordinal_type, value_type>& x);
    void minusEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
                    const OrthogPolyApprox<ordinal_type, value_type>& x);
    void timesEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
                    const OrthogPolyApprox<ordinal_type, value_type>& x);
    void divideEqual(OrthogPolyApprox<ordinal_type, value_type>& c, 
                     const OrthogPolyApprox<ordinal_type, value_type>& x);

    void plus(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a, 
              const OrthogPolyApprox<ordinal_type, value_type>& b);
    void plus(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const value_type& a, 
              const OrthogPolyApprox<ordinal_type, value_type>& b);
    void plus(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a, 
              const value_type& b);
    void minus(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a,
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void minus(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const value_type& a, 
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void minus(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a, 
               const value_type& b);
    void times(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a, 
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void times(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const value_type& a, 
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void times(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a, 
               const value_type& b);
    void divide(OrthogPolyApprox<ordinal_type, value_type>& c, 
                const OrthogPolyApprox<ordinal_type, value_type>& a, 
                const OrthogPolyApprox<ordinal_type, value_type>& b);
    void divide(OrthogPolyApprox<ordinal_type, value_type>& c, 
                const value_type& a, 
                const OrthogPolyApprox<ordinal_type, value_type>& b);
    void divide(OrthogPolyApprox<ordinal_type, value_type>& c, 
                const OrthogPolyApprox<ordinal_type, value_type>& a, 
                const value_type& b);

    void exp(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void log(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void log10(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a);
    void sqrt(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void pow(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a, 
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void pow(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const value_type& a, 
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void pow(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a, 
             const value_type& b);
    void cos(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void sin(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void tan(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void cosh(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void sinh(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void tanh(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void acos(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void asin(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void atan(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void atan2(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a,
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void atan2(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const value_type& a, 
               const OrthogPolyApprox<ordinal_type, value_type>& b);
    void atan2(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a, 
               const value_type& b);
    void acosh(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a);
    void asinh(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a);
    void atanh(OrthogPolyApprox<ordinal_type, value_type>& c, 
               const OrthogPolyApprox<ordinal_type, value_type>& a);
    void abs(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a);
    void fabs(OrthogPolyApprox<ordinal_type, value_type>& c, 
              const OrthogPolyApprox<ordinal_type, value_type>& a);
    void max(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a,
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void max(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const value_type& a, 
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void max(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a, 
             const value_type& b);
    void min(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a,
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void min(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const value_type& a, 
             const OrthogPolyApprox<ordinal_type, value_type>& b);
    void min(OrthogPolyApprox<ordinal_type, value_type>& c, 
             const OrthogPolyApprox<ordinal_type, value_type>& a, 
             const value_type& b);

  private:

    // Prohibit copying
    AlgebraicOrthogPolyExpansion(const AlgebraicOrthogPolyExpansion&);

    // Prohibit Assignment
    AlgebraicOrthogPolyExpansion& operator=(const AlgebraicOrthogPolyExpansion& b);

  protected:

     //! Basis
    Teuchos::RCP<const OrthogPolyBasis<ordinal_type, value_type> > basis;

    //! Triple-product tensor
    Teuchos::RCP<const Stokhos::Sparse3Tensor<ordinal_type, value_type> > Cijk;

    //! Expansions size
    ordinal_type sz;
    
  }; // class AlgebraicOrthogPolyExpansion

} // namespace Stokhos

#include "Stokhos_AlgebraicOrthogPolyExpansionImp.hpp"

#endif // STOKHOS_ALGEBRAICORTHOGPOLYEXPANSION_HPP

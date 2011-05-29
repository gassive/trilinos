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

#include "Teuchos_TestForException.hpp"
#include "Stokhos_DynamicArrayTraits.hpp"

template <typename ordinal_type, typename value_type> 
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
AlgebraicOrthogPolyExpansion(
  const Teuchos::RCP<const Stokhos::OrthogPolyBasis<ordinal_type, value_type> >& basis_,
  const Teuchos::RCP<const Stokhos::Sparse3Tensor<ordinal_type, value_type> >& Cijk_) :
  basis(basis_),
  Cijk(Cijk_),
  sz(basis->size())
{
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
unaryMinus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
	   const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  ordinal_type pc = a.size();
  if (c.size() != pc)
    c.resize(pc);

  value_type* cc = c.coeff();
  const value_type* ca = a.coeff();

  for (ordinal_type i=0; i<pc; i++)
    cc[i] = -ca[i];
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
plusEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
	  const value_type& val)
{
  c[0] += val;
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
minusEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
	   const value_type& val)
{
  c[0] -= val;
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
timesEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
	   const value_type& val)
{
  ordinal_type pc = c.size();
  value_type* cc = c.coeff();
  for (ordinal_type i=0; i<pc; i++)
    cc[i] *= val;
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
divideEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
	    const value_type& val)
{
  ordinal_type pc = c.size();
  value_type* cc = c.coeff();
  for (ordinal_type i=0; i<pc; i++)
    cc[i] /= val;
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
plusEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
          const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& x)
{
  ordinal_type xp = x.size();
  if (c.size() < xp)
    c.resize(xp);

  value_type* cc = c.coeff();
  const value_type* xc = x.coeff();
  for (ordinal_type i=0; i<xp; i++)
    cc[i] += xc[i];
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
minusEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
           const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& x)
{
  ordinal_type xp = x.size();
  if (c.size() < xp)
    c.resize(xp);

  value_type* cc = c.coeff();
  const value_type* xc = x.coeff();
  for (ordinal_type i=0; i<xp; i++)
    cc[i] -= xc[i];
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
timesEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
           const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& x)
{
  ordinal_type p = c.size();
  ordinal_type xp = x.size();
  ordinal_type pc;
  if (p > 1 && xp > 1)
    pc = sz;
  else
    pc = p*xp;
  TEST_FOR_EXCEPTION(sz < pc, std::logic_error,
		     "Stokhos::AlgebraicOrthogPolyExpansion::timesEqual()" <<
		     ":  Expansion size (" << sz << 
		     ") is too small for computation.");
  if (c.size() != pc)
    c.resize(pc);

  value_type* cc = c.coeff();
  const value_type* xc = x.coeff();
  
  if (p > 1 && xp > 1) {
    // Copy c coefficients into temporary array
    value_type* tc = Stokhos::ds_array<value_type>::get_and_fill(cc,p);
    value_type tmp, cijk;
    ordinal_type i,j;
    for (ordinal_type k=0; k<pc; k++) {
      tmp = value_type(0.0);
      ordinal_type n = Cijk->num_values(k);
      for (ordinal_type l=0; l<n; l++) {
	Cijk->value(k,l,i,j,cijk);
	if (i < p && j < xp)
	  tmp += cijk*tc[i]*xc[j];
      }
      cc[k] = tmp / basis->norm_squared(k);
    }
  }
  else if (p > 1) {
    for (ordinal_type i=0; i<p; i++)
      cc[i] *= xc[0];
  }
  else if (xp > 1) {
    for (ordinal_type i=1; i<xp; i++)
      cc[i] = cc[0]*xc[i];
    cc[0] *= xc[0];
  }
  else {
    cc[0] *= xc[0];
  }
}

template <typename ordinal_type, typename value_type> 
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
divideEqual(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
            const Stokhos::OrthogPolyApprox<ordinal_type, value_type >& x)
{
  if (x.size() == 1)
    divideEqual(c, x[0]);
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::divideEqual()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
plus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pa = a.size();
  ordinal_type pb = b.size();
  ordinal_type pc = pa > pb ? pa : pb;
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  if (pa > pb) {
    for (ordinal_type i=0; i<pb; i++)
      cc[i] = ca[i] + cb[i];
    for (ordinal_type i=pb; i<pc; i++)
      cc[i] = ca[i];
  }
  else {
    for (ordinal_type i=0; i<pa; i++)
      cc[i] = ca[i] + cb[i];
    for (ordinal_type i=pa; i<pc; i++)
      cc[i] = cb[i];
  }
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
plus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const value_type& a, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pc = b.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  cc[0] = a + cb[0];
  for (ordinal_type i=1; i<pc; i++)
    cc[i] = cb[i];
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
plus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
     const value_type& b)
{
  ordinal_type pc = a.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  value_type* cc = c.coeff();

  cc[0] = ca[0] + b;
  for (ordinal_type i=1; i<pc; i++)
    cc[i] = ca[i];
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
minus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pa = a.size();
  ordinal_type pb = b.size();
  ordinal_type pc = pa > pb ? pa : pb;
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  if (pa > pb) {
    for (ordinal_type i=0; i<pb; i++)
      cc[i] = ca[i] - cb[i];
    for (ordinal_type i=pb; i<pc; i++)
      cc[i] = ca[i];
  }
  else {
    for (ordinal_type i=0; i<pa; i++)
      cc[i] = ca[i] - cb[i];
    for (ordinal_type i=pa; i<pc; i++)
      cc[i] = -cb[i];
  }
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
minus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const value_type& a, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pc = b.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  cc[0] = a - cb[0];
  for (ordinal_type i=1; i<pc; i++)
    cc[i] = -cb[i];
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
minus(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
      const value_type& b)
{
  ordinal_type pc = a.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  value_type* cc = c.coeff();

  cc[0] = ca[0] - b;
  for (ordinal_type i=1; i<pc; i++)
    cc[i] = ca[i];
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
times(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pa = a.size();
  ordinal_type pb = b.size();
  ordinal_type pc;
  if (pa > 1 && pb > 1)
    pc = sz;
  else
    pc = pa*pb;
  TEST_FOR_EXCEPTION(sz < pc, std::logic_error,
		     "Stokhos::AlgebraicOrthogPolyExpansion::times()" <<
		     ":  Expansion size (" << sz << 
		     ") is too small for computation.");
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  if (pa > 1 && pb > 1) {
    value_type tmp, cijk;
    ordinal_type i,j;
    for (ordinal_type k=0; k<pc; k++) {
      tmp = value_type(0.0);
      ordinal_type n = Cijk->num_values(k);
      for (ordinal_type l=0; l<n; l++) {
    	Cijk->value(k,l,i,j,cijk);
	if (i < pa && j < pb)
	  tmp += cijk*ca[i]*cb[j];
      }
      cc[k] = tmp / basis->norm_squared(k);
    }
  }
  else if (pa > 1) {
    for (ordinal_type i=0; i<pc; i++)
      cc[i] = ca[i]*cb[0];
  }
  else if (pb > 1) {
    for (ordinal_type i=0; i<pc; i++)
      cc[i] = ca[0]*cb[i];
  }
  else {
    cc[0] = ca[0]*cb[0];
  }
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
times(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const value_type& a, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  ordinal_type pc = b.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* cb = b.coeff();
  value_type* cc = c.coeff();

  for (ordinal_type i=0; i<pc; i++)
    cc[i] = a*cb[i];
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
times(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
      const value_type& b)
{
  ordinal_type pc = a.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  value_type* cc = c.coeff();

  for (ordinal_type i=0; i<pc; i++)
    cc[i] = ca[i]*b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
divide(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
       const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
       const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (b.size() == 1)
    divide(c, a, b[0]);
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::divide()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
divide(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
       const value_type& a, 
       const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (b.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = a / b[0];
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::divide()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
divide(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
       const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
       const value_type& b)
{
  ordinal_type pc = a.size();
  if (c.size() != pc)
    c.resize(pc);

  const value_type* ca = a.coeff();
  value_type* cc = c.coeff();

  for (ordinal_type i=0; i<pc; i++)
    cc[i] = ca[i]/b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
exp(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::exp(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::exp()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
log(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::log(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::log()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
log10(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::log10(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::log10()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
sqrt(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::sqrt(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::sqrt()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
pow(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a,
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a.size() == 1 && b.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::pow(a[0], b[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::pow()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
pow(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const value_type& a, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (b.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::pow(a, b[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::pow()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
pow(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
    const value_type& b)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::pow(a[0], b);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::pow()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
sin(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& s, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (s.size() != 1)
      s.resize(1);
    s[0] = std::sin(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::sin()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
cos(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::cos(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::cos()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
tan(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& t, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (t.size() != 1)
      t.resize(1);
    t[0] = std::tan(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::tan()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
sinh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& s, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (s.size() != 1)
      s.resize(1);
    s[0] = std::sinh(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::sinh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
cosh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::cosh(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::cosh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
tanh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& t, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (t.size() != 1)
      t.resize(1);
    t[0] = std::tanh(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::tanh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
acos(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::acos(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::acos()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
asin(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::asin(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::asin()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
atan(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::atan(a[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::atan()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
atan2(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a,
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a.size() == 1 && b.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::atan2(a[0], b[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::atan2()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
atan2(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const value_type& a, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (b.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::atan2(a, b[0]);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::atan2()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
atan2(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
      const value_type& b)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::atan2(a[0], b);
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::atan2()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
acosh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::log(a[0]+std::sqrt(a[0]*a[0]-value_type(1.0)));
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::acosh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
asinh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = std::log(a[0]+std::sqrt(a[0]*a[0]+value_type(1.0)));
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::asinh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
atanh(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
      const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a.size() == 1) {
    if (c.size() != 1)
      c.resize(1);
    c[0] = 0.5*std::log((value_type(1.0)+a[0])/(value_type(1.0)-a[0]));
  }
  else
    TEST_FOR_EXCEPTION(true, std::logic_error,
		       "Stokhos::AlgebraicOrthogPolyExpansion::atanh()" 
		       << ":  Method not implemented!");
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
fabs(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
     const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a[0] >= 0)
    c = a;
  else
    unaryMinus(c,a);
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
abs(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a)
{
  if (a[0] >= 0)
    c = a;
  else
    unaryMinus(c,a);
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
max(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a,
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a[0] >= b[0])
    c = a;
  else
    c = b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
max(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const value_type& a, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a >= b[0]) {
    c = OrthogPolyApprox<ordinal_type, value_type>(basis);
    c[0] = a;
  }
  else
    c = b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
max(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
    const value_type& b)
{
  if (a[0] >= b)
    c = a;
  else {
    c = OrthogPolyApprox<ordinal_type, value_type>(basis);
    c[0] = b;
  }
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
min(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a,
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a[0] <= b[0])
    c = a;
  else
    c = b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
min(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const value_type& a, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& b)
{
  if (a <= b[0]) {
    c = OrthogPolyApprox<ordinal_type, value_type>(basis);
    c[0] = a;
  }
  else
    c = b;
}

template <typename ordinal_type, typename value_type>
void
Stokhos::AlgebraicOrthogPolyExpansion<ordinal_type, value_type>::
min(Stokhos::OrthogPolyApprox<ordinal_type, value_type>& c, 
    const Stokhos::OrthogPolyApprox<ordinal_type, value_type>& a, 
    const value_type& b)
{
  if (a[0] <= b)
    c = a;
  else {
    c = OrthogPolyApprox<ordinal_type, value_type>(basis);
    c[0] = b;
  }
}

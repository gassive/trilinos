// @HEADER
// ***********************************************************************
// 
// Moocho: Multi-functional Object-Oriented arCHitecture for Optimization
//                  Copyright (2003) Sandia Corporation
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
// Questions? Contact Roscoe A. Bartlett (rabartl@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#include <math.h>

#include "NLPInterfacePack_NLPWBCounterExample.hpp"
#include "DenseLinAlgPack_PermVecMat.hpp"
#include "DenseLinAlgPack_LinAlgOpPack.hpp"
#include "Teuchos_TestForException.hpp"

namespace NLPInterfacePack {

// Constructors / initializers

NLPWBCounterExample::NLPWBCounterExample(
  value_type xinit[3], value_type a, value_type b, bool nlp_selects_basis, bool linear_obj
  )
  :is_initialized_(false),nlp_selects_basis_(nlp_selects_basis),basis_selection_was_given_(false)
  ,linear_obj_(linear_obj),n_orig_(3),m_orig_(2),Gc_orig_nz_(4),a_(a),b_(b)
{
#ifdef TEUCHOS_DEBUG	
  const char msg_err_head[] = "NLPWBCounterExample::NLPWBCounterExample(...) : Error";
  TEST_FOR_EXCEPTION( !(b >= 0),        std::invalid_argument, msg_err_head<<"!" );
  TEST_FOR_EXCEPTION( !(a + b*b != 0),  std::invalid_argument, msg_err_head<<"!" );
  TEST_FOR_EXCEPTION( !(xinit[1] >= 0), std::invalid_argument, msg_err_head<<"!" );
  TEST_FOR_EXCEPTION( !(xinit[2] >= 0), std::invalid_argument, msg_err_head<<"!" );
#endif
  // Set the inital guess and the variable bounds
  xinit_orig_.resize(n_orig_); xl_orig_.resize(n_orig_); xu_orig_.resize(n_orig_);
  const value_type inf = NLP::infinite_bound();
  xinit_orig_(1) = xinit[0]; xl_orig_(1) = -inf; xu_orig_(1) = +inf;
  xinit_orig_(2) = xinit[1]; xl_orig_(2) =  0.0; xu_orig_(2) = +inf;
  xinit_orig_(3) = xinit[2]; xl_orig_(3) =  0.0; xu_orig_(3) = +inf;
}

// Overridden public members from NLP

void NLPWBCounterExample::initialize(bool test_setup)
{
  // Nothing to initialize locally so just pass it on
  NLPSerialPreprocessExplJac::initialize(test_setup);
  is_initialized_ = true;
}

bool NLPWBCounterExample::is_initialized() const
{
  return is_initialized_;
}

value_type NLPWBCounterExample::max_var_bounds_viol() const
{
  return +1e+20; // Functions defined everywhere!
}

// Overridden protected methods from NLPSerialPreprocess

bool NLPWBCounterExample::imp_nlp_has_changed() const
{
  return !is_initialized_;
}

size_type NLPWBCounterExample::imp_n_orig() const
{
  return n_orig_;
}

size_type NLPWBCounterExample::imp_m_orig() const
{
  return m_orig_;
}

size_type NLPWBCounterExample::imp_mI_orig() const
{
  return 0;
}

const DVectorSlice NLPWBCounterExample::imp_xinit_orig() const
{
  return xinit_orig_();
}

bool NLPWBCounterExample::imp_has_var_bounds() const
{
  return true;
}

const DVectorSlice NLPWBCounterExample::imp_xl_orig() const
{
  return xl_orig_();
}

const DVectorSlice NLPWBCounterExample::imp_xu_orig() const
{
  return xu_orig_();
}

const DVectorSlice NLPWBCounterExample::imp_hl_orig() const
{
  TEST_FOR_EXCEPT(true);  // Should never be called
   return xinit_orig_();
}

const DVectorSlice NLPWBCounterExample::imp_hu_orig() const
{
  TEST_FOR_EXCEPT(true);  // Should never be called
   return xinit_orig_();
}

void NLPWBCounterExample::imp_calc_f_orig(
  const DVectorSlice &x_full, bool newx, const ZeroOrderInfoSerial &zero_order_info ) const
{
  DVectorSlice x = x_full(1,n_orig_);
  *zero_order_info.f = ( linear_obj_ ? x(1) : 0.5*x(1)*x(1) );
}

void NLPWBCounterExample::imp_calc_c_orig(
  const DVectorSlice &x_full, bool newx, const ZeroOrderInfoSerial &zero_order_info ) const
{
  DVectorSlice x = x_full(1,n_orig_);
  DVector &c = *zero_order_info.c;
  c(1) = x(1)*x(1) - x(2) + a_;
  c(2) = x(1)      - x(3) - b_;
}

void NLPWBCounterExample::imp_calc_h_orig(
  const DVectorSlice &x_full, bool newx, const ZeroOrderInfoSerial &zero_order_info ) const
{
  TEST_FOR_EXCEPT(true); // Should never be called
}

void NLPWBCounterExample::imp_calc_Gf_orig(
  const DVectorSlice &x_full, bool newx, const ObjGradInfoSerial &obj_grad_info ) const
{
  DVectorSlice x = x_full(1,n_orig_); 
  DVector &Gf = *obj_grad_info.Gf;
  Gf(1) = (linear_obj_ ? 1.0 : x(1) ); Gf(2) = 0.0; Gf(3) = 0.0;
}

bool NLPWBCounterExample::imp_get_next_basis(
  IVector      *var_perm_full
  ,IVector     *equ_perm_full
  ,size_type   *rank_full
  ,size_type   *rank
  )
{
#ifdef TEUCHOS_DEBUG
  TEST_FOR_EXCEPT( !( var_perm_full) );
  TEST_FOR_EXCEPT( !( equ_perm_full ) );
  TEST_FOR_EXCEPT( !( rank_full ) );
  TEST_FOR_EXCEPT( !( rank  ) );
#endif
  if(basis_selection_was_given_) return false; // Already gave this basis selection
  // Select x(2) ans x(3) as the basic variables (sorted!)
  var_perm_full->resize(n_orig_);
  (*var_perm_full)(1) = 2;  // The basis variables
  (*var_perm_full)(2) = 3;  // ""
  (*var_perm_full)(3) = 1;  // The nonbasis variable
  equ_perm_full->resize(m_orig_);
  DenseLinAlgPack::identity_perm( equ_perm_full ); // Gc_orig is full rank
  *rank_full                 = m_orig_;
  *rank                      = m_orig_;
  basis_selection_was_given_ = true;
  return true;
}

void NLPWBCounterExample::imp_report_orig_final_solution(
  const DVectorSlice      &x_orig
  ,const DVectorSlice     *lambda_orig
  ,const DVectorSlice     *lambdaI_orig
  ,const DVectorSlice     *nu_orig
  ,bool                   is_optimal
  ) const
{
  // ToDo: Do something with the final soltuion?
}

bool NLPWBCounterExample::nlp_selects_basis() const
{
  return nlp_selects_basis_;
}

// Overridden protected methods from NLPSerialPreprocessExplJac

size_type NLPWBCounterExample::imp_Gc_nz_orig() const
{
  return Gc_orig_nz_;
}

size_type NLPWBCounterExample::imp_Gh_nz_orig() const
{
  return 0;
}

void NLPWBCounterExample::imp_calc_Gc_orig(
  const DVectorSlice& x_full, bool newx, const FirstOrderExplInfo& first_order_expl_info ) const
{
  DVectorSlice x = x_full(1,n_orig_); 
  // Get references/pointers to data for Gc to be computed/updated.
  index_type  &Gc_nz = *first_order_expl_info.Gc_nz;
  value_type	*Gc_v = &(*first_order_expl_info.Gc_val)[0];
  index_type  *Gc_i = ( first_order_expl_info.Gc_ivect ? &(*first_order_expl_info.Gc_ivect)[0] : NULL );
  index_type  *Gc_j = ( first_order_expl_info.Gc_jvect ? &(*first_order_expl_info.Gc_jvect)[0] : NULL );
  // Set up the nonzero structure of Gc_orig (sorted by constraint and then by variable)
  if( Gc_i ) {
    Gc_j[0] = 1; Gc_i[0] = 1; // d(c(1))/d(x(1))
    Gc_j[1] = 1; Gc_i[1] = 2; // d(c(1))/d(x(2))
    Gc_j[2] = 2; Gc_i[2] = 1; // d(c(2))/d(x(1))
    Gc_j[3] = 2; Gc_i[3] = 3; // d(c(2))/d(x(3))
  }
  // Fill in the nonzero values of Gc_orig (must have the same order as structure!)
  Gc_v[0] = 2*x(1); // d(c(1))/d(x(1))
  Gc_v[1] =   -1.0; // d(c(1))/d(x(2))
  Gc_v[2] =   +1.0; // d(c(2))/d(x(1))
  Gc_v[3] =   -1.0; // d(c(2))/d(x(3))
  // Set the actual number of nonzeros
  Gc_nz = Gc_orig_nz_;
}

void NLPWBCounterExample::imp_calc_Gh_orig(
  const DVectorSlice& x_full, bool newx, const FirstOrderExplInfo& first_order_expl_info ) const
{
  TEST_FOR_EXCEPT(true); // Should never be called
}

}	// end namespace NLPInterfacePack
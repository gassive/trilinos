// $Id$ 
// $Source$ 
// @HEADER
// ***********************************************************************
// 
//                           Sacado Package
//                 Copyright (2009) Sandia Corporation
// 
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
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

#include <iostream>
#include <iomanip>

#include "Stokhos.hpp"
#include "Stokhos_StieltjesPCEBasis.hpp"
#include "Stokhos_UserDefinedQuadrature.hpp"

typedef Stokhos::LegendreBasis<int,double> basis_type;

struct pce_quad_func {
  pce_quad_func(
   const Stokhos::OrthogPolyApprox<int,double>& pce_,
   const Stokhos::OrthogPolyBasis<int,double>& basis_) :
    pce(pce_), basis(basis_), vec(2) {}
  
  double operator() (const double& a, const double& b) const {
    vec[0] = a;
    vec[1] = b;
    return pce.evaluate(vec);
  }
  const Stokhos::OrthogPolyApprox<int,double>& pce;
  const Stokhos::OrthogPolyBasis<int,double>& basis;
  mutable Teuchos::Array<double> vec;
};

double rel_err(double a, double b) {
  return std::abs(a-b)/std::abs(b);
}

int main(int argc, char **argv)
{
  try {

    const unsigned int d = 2;
    const unsigned int pmin = 1;
    const unsigned int pmax = 10;
    const unsigned int np = pmax-pmin+1;
    bool use_pce_quad_points = false;
    bool normalize = false;
    bool project_integrals = true;
    Teuchos::Array<double> mean(np), mean_st(np), std_dev(np), std_dev_st(np);

    Teuchos::Array< Teuchos::RCP<const Stokhos::OneDOrthogPolyBasis<int,double> > > bases(d); 
    
    // Loop over orders
    unsigned int n = 0;
    for (unsigned int p=pmin; p<=pmax; p++) {

      std::cout << "p = " << p << std::endl;
      
      // Create product basis
      for (unsigned int i=0; i<d; i++)
	bases[i] = Teuchos::rcp(new basis_type(p));
      Teuchos::RCP<const Stokhos::CompletePolynomialBasis<int,double> > basis = 
	Teuchos::rcp(new Stokhos::CompletePolynomialBasis<int,double>(bases));
      
      // Create approximation
      Stokhos::OrthogPolyApprox<int,double> x(basis), u(basis), v(basis), 
	w(basis), w2(basis);
      for (unsigned int i=0; i<d; i++) {
	x.term(i, 1) = 1.0;
      }
      
      // Tensor product quadrature
      Teuchos::RCP<const Stokhos::Quadrature<int,double> > quad = 
      	Teuchos::rcp(new Stokhos::TensorProductQuadrature<int,double>(basis));
      // Teuchos::RCP<const Stokhos::Quadrature<int,double> > quad = 
      // 	Teuchos::rcp(new Stokhos::SparseGridQuadrature<int,double>(basis, p));

      // Triple product tensor
      Teuchos::RCP<Stokhos::Sparse3Tensor<int,double> > Cijk =
	basis->computeTripleProductTensor(basis->size());
      
      // Quadrature expansion
      Stokhos::QuadOrthogPolyExpansion<int,double> quad_exp(basis, Cijk, quad);
      
      // Compute PCE via quadrature expansion
      quad_exp.sin(u,x);
      quad_exp.exp(v,x);
      quad_exp.times(w,u,v);
	
      // Compute Stieltjes basis
      Teuchos::Array< Teuchos::RCP<const Stokhos::OneDOrthogPolyBasis<int,double> > > st_bases(2);
      st_bases[0] = 
	Teuchos::rcp(new Stokhos::StieltjesPCEBasis<int,double>(
		       p, Teuchos::rcp(&u,false), quad, use_pce_quad_points,
		       normalize, project_integrals, Cijk));
      st_bases[1] = 
	Teuchos::rcp(new Stokhos::StieltjesPCEBasis<int,double>(
		       p, Teuchos::rcp(&v,false), quad, use_pce_quad_points,
		       normalize, project_integrals, Cijk));
      Teuchos::RCP<const Stokhos::CompletePolynomialBasis<int,double> > 
	st_basis = 
	Teuchos::rcp(new Stokhos::CompletePolynomialBasis<int,double>(st_bases));
      //std::cout << *st_basis << std::endl;

      Stokhos::OrthogPolyApprox<int,double>  u_st(st_basis), v_st(st_basis),
	w_st(st_basis);
      u_st.term(0, 0) = u.mean();
      u_st.term(0, 1) = 1.0;
      v_st.term(0, 0) = v.mean();
      v_st.term(1, 1) = 1.0;
      
      // Triple product tensor
      Teuchos::RCP<Stokhos::Sparse3Tensor<int,double> > st_Cijk =
	st_basis->computeTripleProductTensor(st_basis->size());
	
      // Tensor product quadrature
      Teuchos::RCP<const Stokhos::Quadrature<int,double> > st_quad;
      if (!use_pce_quad_points) {
	st_quad = Teuchos::rcp(new Stokhos::TensorProductQuadrature<int,double>(st_basis));
	//st_quad = Teuchos::rcp(new Stokhos::SparseGridQuadrature<int,double>(st_basis, p));
      }
      else {
	Teuchos::Array<double> st_points_0;
	Teuchos::Array<double> st_weights_0;
	Teuchos::Array< Teuchos::Array<double> > st_values_0;
	st_bases[0]->getQuadPoints(p+1, st_points_0, st_weights_0, st_values_0);
	Teuchos::Array<double> st_points_1;
	Teuchos::Array<double> st_weights_1;
	Teuchos::Array< Teuchos::Array<double> > st_values_1;
	st_bases[1]->getQuadPoints(p+1, st_points_1, st_weights_1, st_values_1);
	Teuchos::RCP< Teuchos::Array< Teuchos::Array<double> > > st_points =
	  Teuchos::rcp(new Teuchos::Array< Teuchos::Array<double> >(st_points_0.size()));
	for (int i=0; i<st_points_0.size(); i++) {
	  (*st_points)[i].resize(2);
	  (*st_points)[i][0] = st_points_0[i];
	  (*st_points)[i][1] = st_points_1[i];
	}
	Teuchos::RCP< Teuchos::Array<double> > st_weights = 
	  Teuchos::rcp(new Teuchos::Array<double>(st_weights_0));
	Teuchos::RCP< const Stokhos::OrthogPolyBasis<int,double> > st_b = 
	  st_basis;
	st_quad = 
	  Teuchos::rcp(new Stokhos::UserDefinedQuadrature<int,double>(st_b,
								      st_points,
								      st_weights));
      }
      
      // Quadrature expansion
      Stokhos::QuadOrthogPolyExpansion<int,double> st_quad_exp(st_basis, 
							       st_Cijk,
							       st_quad);
      
      // Compute w_st = u_st*v_st in Stieltjes basis
      st_quad_exp.times(w_st, u_st, v_st);
      
      // Project w_st back to original basis
      pce_quad_func st_func(w_st, *st_basis);
      quad_exp.binary_op(st_func, w2, u, v);

      // std::cout.precision(12);
      // std::cout << w;
      // std::cout << w2;
      // std::cout << w_st;
      mean[n] = w.mean();
      mean_st[n] = w2.mean();
      std_dev[n] = w.standard_deviation();
      std_dev_st[n] = w2.standard_deviation();
      n++;
    }

    n = 0;
    std::cout << "Statistical error:" << std::endl;
    std::cout << "p\t" 
	      << std::setw(10) << "mean" << "\t" 
	      << std::setw(10) << "mean_st" << "\t"
	      << std::setw(10) << "std_dev" << "\t"
	      << std::setw(10) << "std_dev_st" << std::endl;
    for (unsigned int p=pmin; p<pmax; p++) {
      std::cout.precision(3);
      std::cout.setf(std::ios::scientific);
      std::cout << p << "\t" 
		<< std::setw(10) << rel_err(mean[n], mean[np-1]) << "\t"
		<< std::setw(10) << rel_err(mean_st[n], mean[np-1]) << "\t"
		<< std::setw(10) << rel_err(std_dev[n], std_dev[np-1]) << "\t"
		<< std::setw(10) << rel_err(std_dev_st[n], std_dev[np-1]) 
		<< std::endl;
      n++;
    }
      
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

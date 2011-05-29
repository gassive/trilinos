// @HEADER
// ************************************************************************
// 
//        Phalanx: A Partial Differential Equation Field Evaluation 
//       Kernel for Flexible Management of Complex Dependency Chains
//                  Copyright (2008) Sandia Corporation
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
// 
// Questions? Contact Roger Pawlowski (rppawlo@sandia.gov), Sandia
// National Laboratories.
// 
// ************************************************************************
// @HEADER

#ifndef PHX_EXAMPLE_VP_FOURIER_HPP
#define PHX_EXAMPLE_VP_FOURIER_HPP

#include "Phalanx_Evaluator_Macros.hpp"
#include "Phalanx_Field.hpp"

PHX_EVALUATOR_CLASS(Fourier)

  PHX::Field< MyVector<ScalarT> > flux;
  PHX::Field< ScalarT > density;
  PHX::Field< ScalarT > dc;
  PHX::Field< MyVector<ScalarT> > grad_temp;
  
  std::size_t cell_data_size;

PHX_EVALUATOR_CLASS_END

#include "Evaluator_EnergyFlux_Fourier_Def.hpp"

#endif

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

#ifndef PHX_SCALAR_CONTAINER_TEMPLATE_MANAGER_HPP
#define PHX_SCALAR_CONTAINER_TEMPLATE_MANAGER_HPP

#include "Phalanx_TemplateManager.hpp"
#include "Phalanx_EvaluationContainer.hpp"

#include "boost/mpl/placeholders.hpp"
using namespace boost::mpl::placeholders;

namespace PHX {

  template<typename Traits>
  class EvaluationContainer_TemplateManager : 
    public PHX::TemplateManager<typename Traits::EvalTypes,
				PHX::EvaluationContainerBase<Traits>,
				PHX::EvaluationContainer<_,Traits> > {

  public:

    EvaluationContainer_TemplateManager() {}

    ~EvaluationContainer_TemplateManager() {}

  };

} 

#endif 

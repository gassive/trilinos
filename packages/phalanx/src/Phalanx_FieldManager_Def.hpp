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

#ifndef PHX_FIELD_MANAGER_DEF_HPP
#define PHX_FIELD_MANAGER_DEF_HPP

#include "Teuchos_TestForException.hpp"
#include "Sacado_mpl_size.hpp"
#include "Sacado_mpl_find.hpp"
#include "boost/mpl/at.hpp"
#include "Phalanx_EvaluationContainer_TemplateBuilder.hpp"
#include <sstream>

// **************************************************************
template<typename Traits>
inline
PHX::FieldManager<Traits>::FieldManager()
{
  m_num_evaluation_types = 
    Sacado::mpl::size<typename Traits::EvalTypes>::value;
  
  PHX::EvaluationContainer_TemplateBuilder<Traits> builder;
  m_eval_containers.buildObjects(builder);
}

// **************************************************************
template<typename Traits>
inline
PHX::FieldManager<Traits>::~FieldManager()
{ }

// **************************************************************
template<typename Traits>
template<typename DataT, typename EvalT> 
inline
void PHX::FieldManager<Traits>::
getFieldData(PHX::Field<DataT>& f)
{
  f.setFieldData(m_eval_containers.template 
    getAsObject<EvalT>()->template getFieldData<DataT>(f.fieldTag()) );
}
    
// **************************************************************
template<typename Traits>
template<typename DataT, typename EvalT,
	 typename Tag0, typename Tag1, typename Tag2, typename Tag3,
	 typename Tag4, typename Tag5, typename Tag6, typename Tag7> 
inline
void PHX::FieldManager<Traits>::
getFieldData(PHX::MDField<DataT,Tag0,Tag1,Tag2,Tag3,Tag4,
	     Tag5,Tag6,Tag7>& f)
{
  f.setFieldData(m_eval_containers.template 
    getAsObject<EvalT>()->template getFieldData<DataT>(f.fieldTag()) );
}
    
// **************************************************************
template<typename Traits>
template<typename DataT, typename EvalT> 
inline
void PHX::FieldManager<Traits>::
getFieldData(const PHX::FieldTag& t, Teuchos::ArrayRCP<DataT>& d)
{
  d = m_eval_containers.template 
    getAsObject<EvalT>()->template getFieldData<DataT>(t);
}

// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::
requireFieldForAllEvaluationTypes(const PHX::FieldTag& t)
{
  typedef PHX::EvaluationContainer_TemplateManager<Traits> SCTM;
  
  typename SCTM::iterator it = m_eval_containers.begin();
  for (; it != m_eval_containers.end(); ++it) {
    it->requireField(t);
  }
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
requireField(const PHX::FieldTag& t)
{
  m_eval_containers.template getAsBase<EvalT>()->requireField(t);
}
    
// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::
registerEvaluatorForAllEvaluationTypes(const Teuchos::RCP<PHX::Evaluator<Traits> >& e)
{
  typedef PHX::EvaluationContainer_TemplateManager<Traits> SCTM;
  
  typename SCTM::iterator it = m_eval_containers.begin();
  for (; it != m_eval_containers.end(); ++it) {
    it->registerEvaluator(e);
  }
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
registerEvaluator(const Teuchos::RCP<PHX::Evaluator<Traits> >& e)
{
  m_eval_containers.template getAsBase<EvalT>()->registerEvaluator(e);
}

// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::
registerEvaluator(FieldManager::iterator it,
		  const Teuchos::RCP<PHX::Evaluator<Traits> >& e)
{
  it->registerEvaluator(e);
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
postRegistrationSetupForType(typename Traits::SetupData d)
{
//   std::size_t index = 
//     Sacado::mpl::find<typename Traits::EvalTypes,EvalT>::value;
  // boost equivalent of above statement:
  //unsigned index = 
  //  boost::mpl::find<typename Traits::EvalTypes,EvalT>::type::pos::value;

  m_eval_containers.template getAsObject<EvalT>()->
    postRegistrationSetup(d, *this);
}

// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::
postRegistrationSetup(typename Traits::SetupData d)
{
  typedef PHX::EvaluationContainer_TemplateManager<Traits> SCTM;
  typename SCTM::iterator it = m_eval_containers.begin();
  for (std::size_t i = 0; it != m_eval_containers.end(); ++it, ++i)
    it->postRegistrationSetup(d, *this);
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
evaluateFields(typename Traits::EvalData d)
{
  m_eval_containers.template getAsBase<EvalT>()->evaluateFields(d);
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
preEvaluate(typename Traits::PreEvalData d)
{
  m_eval_containers.template getAsBase<EvalT>()->preEvaluate(d);
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
postEvaluate(typename Traits::PostEvalData d)
{
  m_eval_containers.template getAsBase<EvalT>()->postEvaluate(d);
}

// **************************************************************
template<typename Traits>
inline
typename PHX::FieldManager<Traits>::iterator 
PHX::FieldManager<Traits>::begin()
{
  return m_eval_containers.begin();
}

// **************************************************************
template<typename Traits>
inline
typename PHX::FieldManager<Traits>::iterator 
PHX::FieldManager<Traits>::end()
{
  return m_eval_containers.end();
}

// **************************************************************
template<typename Traits>
template<typename EvalT>
inline
void PHX::FieldManager<Traits>::
writeGraphvizFile(const std::string filename,
		  bool writeEvaluatedFields,
		  bool writeDependentFields,
		  bool debugRegisteredEvaluators) const
{
  m_eval_containers.template getAsBase<EvalT>()->
    writeGraphvizFile(filename, writeEvaluatedFields,
		      writeDependentFields, debugRegisteredEvaluators);
}

// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::
writeGraphvizFile(const std::string base_filename, 
		  const std::string file_extension,
		  bool writeEvaluatedFields,
		  bool writeDependentFields,
		  bool debugRegisteredEvaluators) const 
{
  typedef PHX::EvaluationContainer_TemplateManager<Traits> SCTM;
  typename SCTM::const_iterator it = m_eval_containers.begin();
  for (; it != m_eval_containers.end(); ++it) {
    std::string name = base_filename + "_" + it->evaluationType() +
      file_extension;
    it->writeGraphvizFile(name, writeEvaluatedFields, writeDependentFields,
			  debugRegisteredEvaluators);
  }
}

// **************************************************************
template<typename Traits>
inline
void PHX::FieldManager<Traits>::print(std::ostream& os) const
{
  typedef PHX::EvaluationContainer_TemplateManager<Traits> SCTM;
  typename SCTM::const_iterator it = m_eval_containers.begin();
  for (; it != m_eval_containers.end(); ++it)
    os << (*it);
}

// **************************************************************
template<typename Traits>
inline
std::ostream& PHX::operator<<(std::ostream& os, 
			      const PHX::FieldManager<Traits>& vm)
{
  vm.print(os);
  return os;
}

// **************************************************************

#endif

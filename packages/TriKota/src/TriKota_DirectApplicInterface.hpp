// @HEADER
// ************************************************************************
// 
//        TriKota: A Trilinos Wrapper for the Dakota Framework
//                  Copyright (2009) Sandia Corporation
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
// Questions? Contact Andy Salinger (agsalin@sandia.gov), Sandia
// National Laboratories.
// 
// ************************************************************************
// @HEADER

#ifndef TRIKOTA_DIRECTAPPLICINTERFACE
#define TRIKOTA_DIRECTAPPLICINTERFACE

#include "DirectApplicInterface.H"
#include "CommandLineHandler.H"
#include "DakotaStrategy.H"
#include "DakotaModel.H"
#include "ParallelLibrary.H"
#include "ProblemDescDB.H"

#include "EpetraExt_ModelEvaluator.h"
#include "Epetra_Vector.h"
#include "Teuchos_RCP.hpp"
#include "Teuchos_TestForException.hpp"

//!  TriKota namespace
namespace TriKota {

/*! \brief Adapter class that transates from a Trilinos interface toa Dakota interface. 
  An object of this class IS a Dakota::DirectApplicInterface
  and wraps an EpetraExt::ModelEvaluator. It can then be passed in
  as the argument to the TriKota::Driver::run method.
*/
class DirectApplicInterface : public Dakota::DirectApplicInterface
{
public:

  //! Constructor that takes the Model Evaluator to wrap

   DirectApplicInterface(Dakota::ProblemDescDB& problem_db_,
                         const Teuchos::RCP<EpetraExt::ModelEvaluator> App_);

  ~DirectApplicInterface() {};

protected:

  //! Virtual function redefinition from Dakota::DirectApplicInterface
  int derived_map_ac(const Dakota::String& ac_name);

  //! Virtual function redefinition from Dakota::DirectApplicInterface
  int derived_map_of(const Dakota::String& of_name);

  //int derived_map_if(const Dakota::String& if_name);

private:

  // Data
    Teuchos::RCP<EpetraExt::ModelEvaluator> App;
    Teuchos::RCP<Epetra_Vector> model_p;
    Teuchos::RCP<Epetra_Vector> model_g;
    Teuchos::RCP<Epetra_MultiVector> model_dgdp;
    unsigned int numParameters;
    unsigned int numResponses;
    bool supportsSensitivities;
    EpetraExt::ModelEvaluator::EDerivativeMultiVectorOrientation orientation;

};

} // namespace TriKota

#endif //TRIKOTA_DIRECTAPPLICINTERFACE

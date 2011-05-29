// @HEADER
// ************************************************************************
// 
//        Piro: Strategy package for embedded analysis capabilitites
//                  Copyright (2010) Sandia Corporation
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

#include <iostream>
#include <string>

#include "MockModelEval_A.hpp"
#include "ObserveSolution_Epetra.hpp"

#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TestForException.hpp"

#include "Piro_ConfigDefs.hpp"

#ifdef Piro_ENABLE_NOX
#include "Piro_Epetra_NOXSolver.hpp"
#include "Piro_Epetra_LOCASolver.hpp"
#endif
#ifdef Piro_ENABLE_Rythmos
#include "Piro_Epetra_RythmosSolver.hpp"
#endif

#include "Piro_Thyra_PerformAnalysis.hpp"
#include "Thyra_EpetraModelEvaluator.hpp"


int main(int argc, char *argv[]) {

  int status=0; // 0 = pass, failures are incremented
  int overall_status=0; // 0 = pass, failures are incremented over multiple tests

  // Initialize MPI and timer
  int Proc=0;
#ifdef HAVE_MPI
  MPI_Init(&argc,&argv);
  double total_time = -MPI_Wtime();
  (void) MPI_Comm_rank(MPI_COMM_WORLD, &Proc);
  MPI_Comm appComm = MPI_COMM_WORLD;
#else
  int appComm=0;
#endif

  using Teuchos::RCP;
  using Teuchos::rcp;

  char* inputFile;
  bool doAll = (argc==1);
  if (argc>1) doAll = !strcmp(argv[1],"-v");


  for (int iTest=0; iTest<3; iTest++) {

    if (doAll) {
      switch (iTest) {
       case 0: inputFile="input_Analysis_Dakota.xml"; break;
       case 1: inputFile="input_Analysis_OptiPack.xml"; break;
       case 2: inputFile="input_Analysis_MOOCHO.xml"; break;
       default : cout << "iTest logic error " << endl; exit(-1);
      }
    }
    else {
      inputFile=argv[1];
      iTest = 999;
    }

    if (Proc==0) 
     cout << "===================================================\n"
          << "======  Running input file "<< iTest <<": "<< inputFile <<"\n"
          << "===================================================\n"
          << endl;

    try {

      // Create (1) a Model Evaluator and (2) a ParameterList
      RCP<EpetraExt::ModelEvaluator> Model = rcp(new MockModelEval_A(appComm));

      // BEGIN Builder
      Teuchos::ParameterList appParams("Application Parameters");
      Teuchos::updateParametersFromXmlFile(inputFile, &appParams);

      Teuchos::ParameterList piroParams = appParams.sublist("Piro");
      Teuchos::ParameterList& analysisParams = appParams.sublist("Analysis");

      // Use these two objects to construct a Piro solved application 
      //   EpetraExt::ModelEvaluator is  base class of all Piro::Epetra solvers
      RCP<EpetraExt::ModelEvaluator> piro;

      std::string& solver = piroParams.get("Piro Solver","NOX");
      const RCP<Teuchos::ParameterList> piroParamsRCP = rcp(&piroParams, false);

#ifdef Piro_ENABLE_NOX
      RCP<NOX::Epetra::Observer> observer = rcp(new ObserveSolution_Epetra());

      if (solver=="NOX")
        piro = rcp(new Piro::Epetra::NOXSolver(piroParamsRCP, Model, observer));
      else if (solver=="LOCA")
        piro = rcp(new Piro::Epetra::LOCASolver(piroParamsRCP, Model, observer));
      else
#endif
#ifdef Piro_ENABLE_Rythmos
      if (solver=="Rythmos")
        piro = rcp(new Piro::Epetra::RythmosSolver(piroParamsRCP, Model));
      else 
#endif
        TEST_FOR_EXCEPTION(true, std::logic_error,
          "Error: Unknown Piro Solver : " << solver);
      // END Builder

      Thyra::EpetraModelEvaluator piroThyra;
      piroThyra.initialize(piro, Teuchos::null);

      RCP< ::Thyra::VectorBase<double> > p;

      // Now call the analysis routine
      status = Piro::Thyra::PerformAnalysis(piroThyra, analysisParams, p);

      if (p != Teuchos::null) {
        // Can post-process results here
         if (Proc==0) cout << 
           "\nPiro_AnalysisDrvier:  Optimum printed above has exact soln = {1,3}" << endl;
      }

    }

    catch (std::exception& e) {
      cout << e.what() << endl;
      status = 10;
    }
    catch (string& s) {
      cout << s << endl;
      status = 20;
    }
    catch (char *s) {
      cout << s << endl;
      status = 30;
    } catch (...) {
      cout << "Caught unknown exception!" << endl;
      status = 40;
    }

    overall_status += status;
  }  // End loop over tests

#ifdef HAVE_MPI
  total_time +=  MPI_Wtime();
  MPI_Barrier(MPI_COMM_WORLD);
  if (Proc==0) cout << "\n\nTOTAL TIME     " 
                    << total_time << endl;
  MPI_Finalize() ;
#endif

  if (Proc==0) {
    if (overall_status==0) 
      cout << "\nTEST PASSED\n" << endl;
    else 
      cout << "\nTEST Failed: " << overall_status << "\n" << endl;
  }

  return status;
}

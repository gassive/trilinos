/*
// @HEADER
// ***********************************************************************
//
//                    Teuchos: Common Tools Package
//                 Copyright (2004) Sandia Corporation
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
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER
*/

#include "Teuchos_CommandLineProcessor.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_StandardCatchMacros.hpp"
#include "Teuchos_Version.hpp"

// Enum for the speed option
enum ESpeed { SPEED_SLOW=-1, SPEED_MEDIUM=0, SPEED_FAST=+1 };

int main(int argc, char* argv[])
{
  Teuchos::GlobalMPISession mpiSession(&argc,&argv);
  const int procRank = Teuchos::GlobalMPISession::getRank();

  Teuchos::oblackholestream blackhole;
  std::ostream &out = ( procRank == 0 ? std::cout : blackhole );

  bool success = true;
  
  try {
    
    out << Teuchos::Teuchos_Version() << std::endl << std::endl;
    
    // Creating an empty command line processor looks like:
    Teuchos::CommandLineProcessor My_CLP;

    My_CLP.setDocString(
      "This example program demonstrates how to use this Teuchos::CommandLineProcessor class\n"
      "to get options from the command-line and print this help messange automatically.\n"
      );
    
    /* To set and option, it must be given a name and default value.  Additionally,
       each option can be given a help std::string.  Although it is not necessary, a help
       std::string aids a users comprehension of the acceptable command line arguments.
       Some examples of setting command line options are:
    */
    // Set an integer command line option.
    int NumIters = 1550;
    My_CLP.setOption("iterations", &NumIters, "Number of iterations");
    // Set a double-precision command line option.
    double Tolerance = 1e-10;
    My_CLP.setOption("tolerance", &Tolerance, "Tolerance");
    // Set a std::string command line option.
    std::string Solver = "GMRES";
    My_CLP.setOption("solver", &Solver, "Linear solver");
    // Set a boolean command line option.    
    bool Precondition = true;
    My_CLP.setOption("precondition","no-precondition",
                     &Precondition,"Preconditioning flag");
    // Set an enumeration command line option
    const int    num_speed_values  = 3;
    const ESpeed speed_opt_values[] = { SPEED_SLOW, SPEED_MEDIUM, SPEED_FAST };
    const char*  speed_opt_names[]  = { "slow",     "medium",     "fast"     };
    ESpeed       Speed = SPEED_MEDIUM;
    My_CLP.setOption(
      "speed", &Speed,
      num_speed_values, speed_opt_values, speed_opt_names,
      "Speed of our solver"
      );

    /* There are also two methods that control the behavior of the
       command line processor.  First, for the command line processor to
       allow an unrecognized a command line option to be ignored (and
       only have a warning printed), use:
    */
    My_CLP.recogniseAllOptions(true);
  
    /* Second, by default, if the parser finds a command line option it
       doesn't recognize or finds the --help option, it will throw an
       std::exception.  If you want prevent a command line processor from
       throwing an std::exception (which is important in this program since
       we don't have an try/catch around this) when it encounters a
       unrecognized option or help is printed, use:
    */
    My_CLP.throwExceptions(false);

    /* We now parse the command line where argc and argv are passed to
       the parse method.  Note that since we have turned off std::exception
       throwing above we had better grab the return argument so that
       we can see what happened and act accordingly.
    */
    Teuchos::CommandLineProcessor::EParseCommandLineReturn
      parseReturn= My_CLP.parse( argc, argv );
    if( parseReturn == Teuchos::CommandLineProcessor::PARSE_HELP_PRINTED ) {
      return 0;
    }
    if( parseReturn != Teuchos::CommandLineProcessor::PARSE_SUCCESSFUL   ) {
      return 1; // Error!
    }
    // Here is where you would use these command line arguments but for this example program
    // we will just print the help message with the new values of the command-line arguments.
    if (procRank == 0)
      out << "\nPrinting help message with new values of command-line arguments ...\n\n";
    My_CLP.printHelpMessage(argv[0],out);

    // Now we will print the option values
    if (procRank == 0) {
      out << "\nPrinting user options after parsing ...\n\n";
      out << "NumIters     = " << NumIters << std::endl;
      out << "Tolerance    = " << Tolerance << std::endl;
      out << "Solver       = \"" << Solver << "\"\n";
      out << "Precondition = " << Precondition << std::endl;
      out << "Speed        = " << Speed << std::endl;
    }

  } // try
  TEUCHOS_STANDARD_CATCH_STATEMENTS(true,std::cerr,success);
  
  if(success)
    out << "\nEnd Result: TEST PASSED" << std::endl;	

  return ( success ? 0 : 1 );
}

//@HEADER
// ************************************************************************
//
//                 Belos: Block Linear Solvers Package
//                  Copyright 2004 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ************************************************************************
//@HEADER

#ifndef BELOS_TYPES_HPP
#define BELOS_TYPES_HPP

/*!	
  \file BelosTypes.hpp
  \brief Collection of types and exceptions used within the Belos solvers.
*/

#include "BelosConfigDefs.hpp"
#include "Teuchos_TestForException.hpp"

namespace Belos {
 
  //! @name Belos Exceptions
  //@{

  /*! \class BelosError 
      \brief An std::exception class parent to all Belos exceptions.
   */
  class BelosError: public std::logic_error {
    public: BelosError(const std::string& what_arg) : std::logic_error(what_arg) {}
  };

  //@}

 
  /*!
    \enum Belos::ETrans
    \brief Enumerated list for describing the application of an operator.
  */
  enum ETrans     {	NOTRANS = 0,  /*!< The operator should not be transposed during this application */
			TRANS = 1,    /*!< The operator should be transposed during this application */
			CONJTRANS = 2 /*!< The operator should be transposed and conjugated during this application */
  };
  
  /*! 	
    \enum Belos::NormType
    \brief Enumerated list for describing the multivector norm type.
  */
  enum NormType {   OneNorm,       /*!< Compute the one-norm \f$\sum_{i=1}^{n}(|x_i w_i|)\f$ for each std::vector. */
		    TwoNorm,       /*!< Compute the two-norm \f$\sqrt(\sum_{i=1}^{n}((x_i w_i)^2))\f$ for each std::vector. */
		    InfNorm        /*!< Compute the infinity-norm \f$\max_{i=1}^{n}\{|x_i w_i|\}\f$ for each std::vector. */
  };
  
  /*!
   \enum Belos::ScaleType 
   \brief Enumerated list for describing the type of scaling used on the residual.
  */
  enum ScaleType {NormOfRHS,     /*!< Use the norm of the right-hand-side. */
                  NormOfInitRes, /*!< Use the initial residual vector. */
                  NormOfPrecInitRes, /*!< Use the preconditioned initial residual vector. */
                  None,          /*!< Use unscaled residual. */
                  UserProvided   /*!< User provides an explicit value that the norm of
                                   the residual will be divided by. */
  };
 
  /*!
   \enum Belos::OutputType 
   \brief Enumerated list for describing the style of output used to display status test information.
  */
  enum OutputType {General,     /*!< Extensive output of status test information. */
                   Brief        /*!< Simple output of current residual information. */
  };

  /*!	
    \enum Belos::ReturnType
    When the solve() method of any Belos::SolverManager is called a variable of type
    Belos::ReturnType is returned indicating whether the solver manager sucessfully computed
    solutions to the linear system.
  */
  
  enum ReturnType  {    Converged,     /*!< Convergence was reached for all linear systems. */
                        Unconverged    /*!< Convergence was not reached for some or all linear systems. */
  };
  
  /*! 
    \enum Belos::StatusType 
    When the CheckStatus and GetStatus methods of Belos::StatusTest objects are called a 
    variable of type Belos::StatusType is returned.
  */
  
  enum StatusType { 	Passed = 0x1,      /*!< Some event occured, the iteration needs to stop. */
                        Failed = 0x2,      /*!< No event has occurred requiring the iteration to stop. */
			Undefined = 0x4    /*!< Status test has not been checked yet. */
  };

  enum ResetType  {     Problem = 0x1,           /*!< Reset the linear problem inside the solver. */
                        RecycleSubspace = 0x2    /*!< Destroy any existing subspace inside the solver. */
  };

  /*!
    Return a std::string name for a StatusType object.
  */
  inline
  const char* toString(const StatusType status)
  {
    switch(status) {
      case Passed:
        return "Passed";
      case Failed:
        return "Failed";
      case Undefined:
        return "Undefined";
      default:
        TEST_FOR_EXCEPT(true);
    }
    return NULL; // Should never be called!
  }
  
  /*! 
    \enum Belos::ConjType
    When the MvTransMv and MvDot methods of Belos::MultiVec are called, the require the knowledge of
    whether the conjugate of the transpose should be computed.
  */
  
  enum ConjType {
    NO_CONJ,      /*!< Not conjugated */
    CONJ          /*!< Conjugated */
  };
  
  /*! \enum MsgType
    \brief Enumerated list of available message types recognized by the linear solvers.
  */
  
  enum MsgType 
    {
      Errors= 0,                  /*!< Errors [ always printed ] */
      Warnings = 0x1,             /*!< Internal warnings */
      IterationDetails = 0x2,     /*!< Approximate/exact residuals */
      OrthoDetails = 0x4,         /*!< Orthogonalization/orthonormalization details */
      FinalSummary = 0x8,         /*!< Final computational summary */
      TimingDetails = 0x10,       /*!< Timing details */
      StatusTestDetails = 0x20,   /*!< Status test details */
      Debug = 0x40                /*!< Debugging information */
    };

} // end Belos namespace

#endif /* BELOS_TYPES_HPP */

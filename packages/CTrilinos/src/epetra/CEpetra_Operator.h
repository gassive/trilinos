
/*! @HEADER */
/*
************************************************************************

                CTrilinos:  C interface to Trilinos
                Copyright (2009) Sandia Corporation

Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
license for use of this work by or on behalf of the U.S. Government.

This library is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation; either version 2.1 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
USA
Questions? Contact M. Nicole Lemaster (mnlemas@sandia.gov)

************************************************************************
*/
/*! @HEADER */


#include "CTrilinos_config.h"


/*! @file CEpetra_Operator.h
 * @brief Wrappers for Epetra_Operator */

/* True C header file! */


#ifndef CEPETRA_OPERATOR_H
#define CEPETRA_OPERATOR_H


#include "CTrilinos_enums.h"


#ifdef __cplusplus
extern "C" {
#endif



/*! @name ID struct conversion functions */
/*@{*/

/*! @brief Changes the ID struct from the universal
   (generalized) struct type to the class-specific one.
*/
CT_Epetra_Operator_ID_t Epetra_Operator_Degeneralize ( 
  CTrilinos_Universal_ID_t id );

/*! @brief Changes the ID struct from the class-specific
   struct type to the universal (generalized) one.
*/
CTrilinos_Universal_ID_t Epetra_Operator_Generalize ( 
  CT_Epetra_Operator_ID_t id );

/*@}*/

/*! @name Epetra_Operator destructor wrappers */
/*@{*/

/*! @brief Wrapper for 
   virtual Epetra_Operator::~Epetra_Operator()
*/
void Epetra_Operator_Destroy ( CT_Epetra_Operator_ID_t * selfID );

/*@}*/

/*! @name Epetra_Operator member wrappers */
/*@{*/

/*! @brief Wrapper for 
   virtual int Epetra_Operator::SetUseTranspose(bool UseTranspose) = 0
*/
int Epetra_Operator_SetUseTranspose ( 
  CT_Epetra_Operator_ID_t selfID, boolean UseTranspose );

/*! @brief Wrapper for 
   virtual int Epetra_Operator::Apply(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const = 0
*/
int Epetra_Operator_Apply ( 
  CT_Epetra_Operator_ID_t selfID, CT_Epetra_MultiVector_ID_t XID, 
  CT_Epetra_MultiVector_ID_t YID );

/*! @brief Wrapper for 
   virtual int Epetra_Operator::ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const = 0
*/
int Epetra_Operator_ApplyInverse ( 
  CT_Epetra_Operator_ID_t selfID, CT_Epetra_MultiVector_ID_t XID, 
  CT_Epetra_MultiVector_ID_t YID );

/*! @brief Wrapper for 
   virtual double Epetra_Operator::NormInf() const = 0
*/
double Epetra_Operator_NormInf ( CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual const char * Epetra_Operator::Label() const = 0
*/
const char * Epetra_Operator_Label ( CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual bool Epetra_Operator::UseTranspose() const = 0
*/
boolean Epetra_Operator_UseTranspose ( 
  CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual bool Epetra_Operator::HasNormInf() const = 0
*/
boolean Epetra_Operator_HasNormInf ( CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual const Epetra_Comm & Epetra_Operator::Comm() const = 0
*/
CT_Epetra_Comm_ID_t Epetra_Operator_Comm ( 
  CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual const Epetra_Map & Epetra_Operator::OperatorDomainMap() const = 0
*/
CT_Epetra_Map_ID_t Epetra_Operator_OperatorDomainMap ( 
  CT_Epetra_Operator_ID_t selfID );

/*! @brief Wrapper for 
   virtual const Epetra_Map & Epetra_Operator::OperatorRangeMap() const = 0
*/
CT_Epetra_Map_ID_t Epetra_Operator_OperatorRangeMap ( 
  CT_Epetra_Operator_ID_t selfID );

/*@}*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CEPETRA_OPERATOR_H */


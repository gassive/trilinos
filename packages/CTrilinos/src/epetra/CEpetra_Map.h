
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


/*! @file CEpetra_Map.h
 * @brief Wrappers for Epetra_Map */

/* True C header file! */


#ifndef CEPETRA_MAP_H
#define CEPETRA_MAP_H


#include "CTrilinos_enums.h"


#ifdef __cplusplus
extern "C" {
#endif



/*! @name ID struct conversion functions */
/*@{*/

/*! @brief Changes the ID struct from the universal
   (generalized) struct type to the class-specific one.
*/
CT_Epetra_Map_ID_t Epetra_Map_Degeneralize ( 
  CTrilinos_Universal_ID_t id );

/*! @brief Changes the ID struct from the class-specific
   struct type to the universal (generalized) one.
*/
CTrilinos_Universal_ID_t Epetra_Map_Generalize ( 
  CT_Epetra_Map_ID_t id );

/*@}*/

/*! @name Epetra_Map constructor wrappers */
/*@{*/

/*! @brief Wrapper for 
   Epetra_Map::Epetra_Map(int NumGlobalElements, int IndexBase, const Epetra_Comm& Comm)
*/
CT_Epetra_Map_ID_t Epetra_Map_Create ( 
  int NumGlobalElements, int IndexBase, CT_Epetra_Comm_ID_t CommID );

/*! @brief Wrapper for 
   Epetra_Map::Epetra_Map(int NumGlobalElements, int NumMyElements, int IndexBase, const Epetra_Comm& Comm)
*/
CT_Epetra_Map_ID_t Epetra_Map_Create_Linear ( 
  int NumGlobalElements, int NumMyElements, int IndexBase, 
  CT_Epetra_Comm_ID_t CommID );

/*! @brief Wrapper for 
   Epetra_Map::Epetra_Map(int NumGlobalElements, int NumMyElements, const int *MyGlobalElements, int IndexBase, const Epetra_Comm& Comm)
*/
CT_Epetra_Map_ID_t Epetra_Map_Create_Arbitrary ( 
  int NumGlobalElements, int NumMyElements, 
  const int * MyGlobalElements, int IndexBase, 
  CT_Epetra_Comm_ID_t CommID );

/*! @brief Wrapper for 
   Epetra_Map::Epetra_Map(const Epetra_Map& map)
*/
CT_Epetra_Map_ID_t Epetra_Map_Duplicate ( CT_Epetra_Map_ID_t mapID );

/*@}*/

/*! @name Epetra_Map destructor wrappers */
/*@{*/

/*! @brief Wrapper for 
   virtual Epetra_Map::~Epetra_Map(void)
*/
void Epetra_Map_Destroy ( CT_Epetra_Map_ID_t * selfID );

/*@}*/

/*! @name Epetra_Map operator wrappers */
/*@{*/

/*! @brief Wrapper for 
   Epetra_Map & Epetra_Map::operator=(const Epetra_Map & map)
*/
void Epetra_Map_Assign ( 
  CT_Epetra_Map_ID_t selfID, CT_Epetra_Map_ID_t mapID );

/*@}*/


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif /* CEPETRA_MAP_H */


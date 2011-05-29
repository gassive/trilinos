
//@HEADER
// ************************************************************************
// 
//               Epetra: Linear Algebra Services Package 
//                 Copyright 2001 Sandia Corporation
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

#include "Epetra_Map.h"


//==============================================================================
// Epetra_Map constructor for a Epetra-defined uniform linear distribution of elements.
Epetra_Map::Epetra_Map(int NumGlobalElements, int IndexBase, const Epetra_Comm& Comm)
  : Epetra_BlockMap(NumGlobalElements, 1, IndexBase, Comm) // Map is just a special case of BlockMap
{
  SetLabel("Epetra::Map");
}
//==============================================================================
// Epetra_Map constructor for a user-defined linear distribution of constant block size elements.
Epetra_Map::Epetra_Map(int NumGlobalElements, int NumMyElements, int IndexBase, const Epetra_Comm& Comm)
  : Epetra_BlockMap(NumGlobalElements, NumMyElements, 1, IndexBase, Comm) // Map is just a special case of BlockMap
{
  SetLabel("Epetra::Map");
}
//==============================================================================
// Epetra_Map constructor for a user-defined arbitrary distribution of constant block size elements.
Epetra_Map::Epetra_Map(int NumGlobalElements, int NumMyElements,
                       const int * MyGlobalElements,
                       int IndexBase, const Epetra_Comm& Comm)
  : Epetra_BlockMap(NumGlobalElements, NumMyElements, MyGlobalElements, 1, IndexBase, Comm) // Map is just a special case of BlockMap
{
  SetLabel("Epetra::Map");
}
//==============================================================================
Epetra_Map::Epetra_Map(const Epetra_Map& map)
  : Epetra_BlockMap(map) // Map is just a special case of BlockMap
{
}

//==============================================================================
Epetra_Map::~Epetra_Map(void)
{
}

//=============================================================================
Epetra_Map & Epetra_Map::operator= (const Epetra_Map& map)
{
  if(this != &map) {
    Epetra_BlockMap::operator=(map); // call this->Epetra_BlockMap::operator=
  }
  return(*this);
}

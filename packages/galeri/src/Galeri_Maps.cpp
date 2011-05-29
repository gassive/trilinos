// @HEADER
// ************************************************************************
//
//           Galeri: Finite Element and Matrix Generation Package
//                 Copyright (2006) ETHZ/Sandia Corporation
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
// Questions about Galeri? Contact Marzio Sala (marzio.sala _AT_ gmail.com)
//
// ************************************************************************
// @HEADER

#include "Galeri_Maps.h"
#include "Galeri_Exception.h"
#include "Galeri_Utils.h"
#include "Epetra_Comm.h"
#include "Epetra_Map.h"
#include "Teuchos_ParameterList.hpp"
#include "Maps/Galeri_Linear.h"
#include "Maps/Galeri_NodeCartesian2D.h"
#include "Maps/Galeri_Cartesian2D.h"
#include "Maps/Galeri_Cartesian3D.h"
#include "Maps/Galeri_Random.h"
#include "Maps/Galeri_Interlaced.h"

namespace Galeri {

Epetra_Map* 
CreateMap(string MapType, Epetra_Comm& Comm, Teuchos::ParameterList& List)
{
  // global parameters
  int n = List.get("n", -1);

  // Cycle on map type //
  if (MapType == "Linear")
  {
    // get the parameters
    return(Maps::Linear(Comm, n));
  }
  else if (MapType == "Interlaced")
  {
    return(Maps::Interlaced(Comm, n));
  }
  else if (MapType == "Random")
  {
    return(Maps::Random(Comm, n));
  }
  else if (MapType == "Cartesian2D")
  {
    // Get matrix dimension
    int nx = List.get("nx", -1);
    int ny = List.get("ny", -1);

    if (nx == -1 || ny == -1) 
    {
      if (n <= 0)
          throw(Exception(__FILE__, __LINE__,
                          "If nx or ny are not set, then n must be set"));

      nx = (int)sqrt((double)n);
      ny = nx;
      if (nx * ny != n) 
        throw(Exception(__FILE__, __LINE__,
                        "The number of global elements (n) must be",
                        "a perfect square, otherwise set nx and ny"));
    }

    // Get the number of domains
    int mx = List.get("mx", -1);
    int my = List.get("my", -1);

    if (mx == -1 || my == -1) 
    {
      mx = (int)(sqrt((double)(Comm.NumProc()+.001)));
      my =  Comm.NumProc()/mx;

      // simple attempt at trying to find an mx and my such 
      // mx*my = NumProc

      while ( (mx*my) != Comm.NumProc() ) {
        mx--;
        my = Comm.NumProc()/mx;
      }
    } 

    return(Maps::Cartesian2D(Comm, nx, ny, mx, my));
  }
  else if (MapType == "NodeCartesian2D")
  {
    // Get matrix dimension
    int nx = List.get("nx", -1);
    int ny = List.get("ny", -1);

    if (nx == -1 || ny == -1) 
    {
      if (n <= 0)
          throw(Exception(__FILE__, __LINE__,
                          "If nx or ny are not set, then n must be set"));

      nx = (int)sqrt((double)n);
      ny = nx;
      if (nx * ny != n) 
        throw(Exception(__FILE__, __LINE__,
                        "The number of global elements (n) must be",
                        "a perfect square, otherwise set nx and ny"));
    }

    // Get the number of nodes
    int NumNodes = List.get("number of nodes",-1);
    int MyNodeID = List.get("node id",-1);
    int ndx = List.get("ndx", -1);
    int ndy = List.get("ndy", -1);
    if (ndx == -1 || ndy == -1) 
    {
      ndx = (int)(sqrt((double)(NumNodes+.001)));
      ndy = NumNodes/ndx;
      while ( (ndx*ndy) != NumNodes ) {
        ndx--;
        ndy = NumNodes/ndx;
      }
    } 

    // Get the number of processors per node
    Epetra_Comm * NodeComm= List.get("node communicator",(Epetra_Comm*)0);
    int px = List.get("px", -1);
    int py = List.get("py", -1);
    if (px == -1 || py == -1) 
    {
      px = (int)(sqrt((double)(NodeComm->NumProc()+.001)));
      py = NodeComm->NumProc()/px;

      // simple attempt at trying to find an ndx and ndy such 
      // ndx*ndy = NumNodes

      while ( (px*py) != NodeComm->NumProc() ) {
        px--;
        py = NodeComm->NumProc()/px;
      }
    } 
  
    return(Maps::NodeCartesian2D(Comm, *NodeComm, MyNodeID, nx, ny, ndx, ndy, px,py));
  }  
  else if (MapType == "Cartesian3D")
  {
    // Get matrix dimension
    int nx = List.get("nx", -1);
    int ny = List.get("ny", -1);
    int nz = List.get("nz", -1);

    if (nx == -1 || ny == -1 || nz == -1) 
    {
      if (n <= 0)
          throw(Exception(__FILE__, __LINE__,
                          "If nx or ny or nz are not set, then n must be set"));

      nx = (int)pow((double)n, 0.333334);
      ny = nx;
      nz = nx;
      if (nx * ny * nz != n) 
        throw(Exception(__FILE__, __LINE__,
                        "The number of global elements n (" +
                        toString(n) + ") must be",
                        "a perfect cube, otherwise set nx, ny and nz"));
    }

    // Get the number of domains
    int mx = List.get("mx", -1);
    int my = List.get("my", -1);
    int mz = List.get("mz", -1);

    if (mx == -1 || my == -1 || mz == -1) 
    {
      mx = (int)pow((double)Comm.NumProc(), 0.333334);
      my = mx;
      mz = mx;

      if (mx * my * mz != Comm.NumProc())  {
	  // simple attempt to find a set of processor assignments
         mx = 1; my = 1; mz = 1;
         int ProcTemp = Comm.NumProc();
         int factors[50];
         for (int jj = 0; jj < 50; jj++) factors[jj] = 0;
         for (int jj = 2; jj < 50; jj++) {
            int flag = 1;
            while (flag == 1) {
               int temp = ProcTemp/jj;
               if (temp*jj == ProcTemp) {
                  factors[jj]++; ProcTemp = temp;
               }
               else flag = 0;
            }
         }
         mx = ProcTemp;
         for (int jj = 50-1; jj > 0; jj--) {
             while (factors[jj] != 0) {
                if (  (mx <= my) && (mx <= mz) ) mx = mx*jj;
                else if (  (my <= mx) && (my <= mz) ) my = my*jj;
                else mz = mz*jj;
                factors[jj]--;
             }
         }
        
      }
    } 
    else 
    {
      if (mx * my * mz != Comm.NumProc()) 
        throw(Exception(__FILE__, __LINE__, 
                        "mx * my * mz != number of processes!",
                        "mx = " + toString(mx) + ", my = " + toString(my)
                        + ", mz = " + toString(mz)));
    }

    return(Maps::Cartesian3D(Comm, nx, ny, nz, mx, my, mz));
  }
  else 
  {
    throw(Exception(__FILE__, __LINE__,
                    "`MapType' has incorrect value (" + MapType + ")",
                    "in input to function CreateMap()",
                    "Check the documentation for a list of valid choices"));
  }
} // CreateMap()

} // namespace Galeri

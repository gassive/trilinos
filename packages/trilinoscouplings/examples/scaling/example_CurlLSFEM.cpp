// @HEADER
// ************************************************************************
//
//                           Intrepid Package
//                 Copyright (2007) Sandia Corporation
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
// Questions? Contact Pavel Bochev  (pbboche@sandia.gov),
//                    Denis Ridzal  (dridzal@sandia.gov),
//                    Kara Peterson (kjpeter@sandia.gov).
//
// ************************************************************************
// @HEADER

/** \file   example_CurlLSFEM.cpp
    \brief  Example solution of a div-curl system on a hexahedral mesh using
            curl-conforming (edge) elements.

            This example uses Pamgen to generate a hexahedral mesh, Intrepid to 
            build mass and stiffness matrices, and ML to solve.

    \verbatim

            Div-Curl System:

                       curl u = g  in Omega
                        div u = h  in Omega
                        u x n = 0  on Gamma

            Corresponding discrete linear system for edge element coeficients (x):

                      (Kc + Mc*Dg*MgInv*Dg'*Mc)x = b

                      Kc    - Hcurl stiffness matrix
                      Mc    - Hcurl mass matrix
                      Dg    - Node to edge incidence matrix
                      MgInv - Hgrad mass matrix inverse
                      b     - right hand side vector

    \endverbatim

    \author Created by P. Bochev, D. Ridzal, K. Peterson, D. Hensinger, C. Siefert.

     \remark Usage
     \verbatim

     ./TrilinosCouplings_examples_scaling_Example_CurlLSFEM.exe  inputfile.xml

        inputfile.xml (optional)  -  xml input file containing Pamgen mesh description
                                     and material parameters for each Pamgen block,
                                     if not present code attempts to read CurlLSFEMin.xml.
                                      

     \endverbatim
 **/


// Intrepid includes
#include "Intrepid_FunctionSpaceTools.hpp"
#include "Intrepid_FieldContainer.hpp"
#include "Intrepid_CellTools.hpp"
#include "Intrepid_ArrayTools.hpp"
#include "Intrepid_HCURL_HEX_I1_FEM.hpp"
#include "Intrepid_HGRAD_HEX_C1_FEM.hpp"
#include "Intrepid_RealSpaceTools.hpp"
#include "Intrepid_DefaultCubatureFactory.hpp"
#include "Intrepid_Utils.hpp"

// Epetra includes
#include "Epetra_Time.h"
#include "Epetra_Map.h"
#ifdef HAVE_MPI
#include "Epetra_MpiComm.h"
#else
#include "Epetra_SerialComm.h"
#endif
#include "Epetra_Import.h"
#include "Epetra_Export.h"
#include "Epetra_FECrsMatrix.h"
#include "Epetra_FEVector.h"
#include "Epetra_LinearProblem.h"

// Teuchos includes
#include "Teuchos_oblackholestream.hpp"
#include "Teuchos_RCP.hpp"
#include "Teuchos_BLAS.hpp"
#include "Teuchos_GlobalMPISession.hpp"
#include "Teuchos_ParameterList.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"

// Shards includes
#include "Shards_CellTopology.hpp"

// EpetraExt includes
#include "EpetraExt_RowMatrixOut.h"
#include "EpetraExt_MultiVectorOut.h"
#include "EpetraExt_VectorOut.h"

// AztecOO includes
#include "AztecOO.h"

// ML Includes
#include "ml_MultiLevelPreconditioner.h"
#include "ml_RefMaxwell_11_Operator.h"
#include "ml_RefMaxwell.h"
#include "ml_EdgeMatrixFreePreconditioner.h"
#include "ml_epetra_utils.h"

// Pamgen includes
#include "create_inline_mesh.h"
#include "im_exodusII_l.h"
#include "im_ne_nemesisI_l.h"
#include "pamgen_extras.h"


#define ABS(x) ((x)>0?(x):-(x))

//#define DUMP_DATA
//#define DUMP_DATAE

using namespace std;
using namespace Intrepid;

struct fecomp{
  bool operator () ( topo_entity* x,  topo_entity*  y )const
  {
    if(x->sorted_local_node_ids < y->sorted_local_node_ids)return true;
    return false;
  }
};

void TestMultiLevelPreconditioner_CurlLSFEM(char ProblemType[],
                                           Teuchos::ParameterList   & MLList,
                                           Epetra_CrsMatrix   & CurlCurl,
                                           Epetra_CrsMatrix   & D0clean,
                                           Epetra_CrsMatrix   & M0inv,
                                           Epetra_CrsMatrix   & M1,
                                           Epetra_MultiVector & xh,
                                           Epetra_MultiVector & b,
                                           double & TotalErrorResidual,
                                             double & TotalErrorExactSol);


// Functions to evaluate exact solution and derivatives
int evalu(double & uExact0, 
          double & uExact1, 
          double & uExact2,  
          double & x, 
          double & y, 
          double & z);

double evalDivu(double & x, 
                double & y, 
                double & z, 
                double & mu);

int evalCurlu(double & curlu0, 
              double & curlu1, 
              double & curlu2, 
              double & x, 
              double & y, 
              double & z,  
              double & mu);

int evalGradDivu(double & gradDivu0, 
                 double & gradDivu1, 
                 double & gradDivu2, 
                 double & x, double & y, 
                 double & z, double & mu);

int main(int argc, char *argv[]) {

  int error = 0;
#ifdef HAVE_MPI
  Teuchos::GlobalMPISession mpiSession(&argc, &argv,0);
  int rank=mpiSession.getRank();
  int numProcs=mpiSession.getNProc();
  Epetra_MpiComm Comm(MPI_COMM_WORLD);
  int MyPID = Comm.MyPID();
#else
  int rank=0;
  int numProcs=1;
  int MyPID = 0;
  Epetra_SerialComm Comm;
#endif
  Epetra_Time Time(Comm);

   //Check number of arguments
  if (argc > 2) {
      std::cout <<"\n>>> ERROR: Invalid number of arguments.\n\n";
      std::cout <<"Usage:\n\n";
      std::cout <<"  ./TrilinosCouplings_examples_scaling_Example_CurlLSFEM.exe [inputfile.xml] \n\n";
      std::cout <<"   inputfile.xml(optional) - xml file with description of Pamgen mesh \n";
      std::cout <<"                             and material parameters for each block \nn";
      exit(1);
   }
  
 if (MyPID == 0) {
  std::cout \
    << "===============================================================================\n" \
    << "|                                                                             |\n" \
    << "|    Example: Solve Div-Curl System on Hexahedral Mesh                        |\n" \
    << "|               with Curl-conforming Elements                                 |\n" \
    << "|                                                                             |\n" \
    << "|  Questions? Contact  Pavel Bochev  (pbboche@sandia.gov),                    |\n" \
    << "|                      Denis Ridzal  (dridzal@sandia.gov),                    |\n" \
    << "|                      Kara Peterson (kjpeter@sandia.gov).                    |\n" \
    << "|                                                                             |\n" \
    << "|  Intrepid's website: http://trilinos.sandia.gov/packages/intrepid           |\n" \
    << "|  Pamgen's website:   http://trilinos.sandia.gov/packages/pamgen             |\n" \
    << "|  ML's website:       http://trilinos.sandia.gov/packages/ml                 |\n" \
    << "|  Trilinos website:   http://trilinos.sandia.gov                             |\n" \
    << "|                                                                             |\n" \
    << "===============================================================================\n";
  }

  long long *  node_comm_proc_ids   = NULL;
  long long *  node_cmap_node_cnts  = NULL;
  long long *  node_cmap_ids        = NULL;
  long long ** comm_node_ids        = NULL;
  long long ** comm_node_proc_ids   = NULL;

  std::set < topo_entity * , fecomp > edge_set;
  std::set < topo_entity * , fecomp > face_set;

  std::vector < topo_entity * > edge_vector;
  std::vector < topo_entity * > face_vector;

  std::vector < int > edge_comm_procs;

// ************************************ GET INPUTS **************************************

  // Command line for xml file, otherwise use default
    std::string   xmlInFileName;
    if(argc>=2) xmlInFileName=string(argv[1]);
    else xmlInFileName="CurlLSFEMin.xml";


  // Read xml file into parameter list
    Teuchos::ParameterList inputList;

   if(xmlInFileName.length()) {
    if (MyPID == 0) {
      std::cout << "\nReading parameter list from the XML file \""<<xmlInFileName<<"\" ...\n\n";
    }
      Teuchos::updateParametersFromXmlFile(xmlInFileName,&inputList);
    if (MyPID == 0) {
      inputList.print(std::cout,2,true,true);
      std::cout << "\n";
     }
    }
    else
    {
      std::cout << "Cannot read input file: " << xmlInFileName << "\n";
      return 0;
    }

  // Get pamgen mesh definition
    std::string meshInput = Teuchos::getParameter<std::string>(inputList,"meshInput");
    

// *********************************** CELL TOPOLOGY **********************************

   // Get cell topology for base hexahedron
    typedef shards::CellTopology    CellTopology;
    CellTopology hex_8(shards::getCellTopologyData<shards::Hexahedron<8> >() );

   // Get dimensions 
    int numNodesPerElem = hex_8.getNodeCount();
    int numEdgesPerElem = hex_8.getEdgeCount();
    int numFacesPerElem = hex_8.getSideCount();
    int numNodesPerEdge = 2;
    int numNodesPerFace = 4;
    int numEdgesPerFace = 4;
    int spaceDim = hex_8.getDimension();

   // Build reference element edge to node map
    FieldContainer<int> refEdgeToNode(numEdgesPerElem,numNodesPerEdge);
    for (int i=0; i<numEdgesPerElem; i++){
        refEdgeToNode(i,0)=hex_8.getNodeMap(1, i, 0);
        refEdgeToNode(i,1)=hex_8.getNodeMap(1, i, 1);
    }

   // Build reference element face to node map
    FieldContainer<int> refFaceToNode(numFacesPerElem,numNodesPerFace);
    for (int i=0; i<numFacesPerElem; i++){
        refFaceToNode(i,0)=hex_8.getNodeMap(2, i, 0);
        refFaceToNode(i,1)=hex_8.getNodeMap(2, i, 1);
        refFaceToNode(i,2)=hex_8.getNodeMap(2, i, 2);
        refFaceToNode(i,3)=hex_8.getNodeMap(2, i, 3);
    }

   // Build reference element face to edge map (Hardcoded for now)
    FieldContainer<int> refFaceToEdge(numFacesPerElem,numEdgesPerFace);
        refFaceToEdge(0,0)=0; refFaceToEdge(0,1)=9; 
        refFaceToEdge(0,2)=4; refFaceToEdge(0,3)=8;
        refFaceToEdge(1,0)=1; refFaceToEdge(1,1)=10;
        refFaceToEdge(1,2)=5; refFaceToEdge(1,3)=9;
        refFaceToEdge(2,0)=2; refFaceToEdge(2,1)=11;
        refFaceToEdge(2,2)=6; refFaceToEdge(2,3)=10;
        refFaceToEdge(3,0)=3; refFaceToEdge(3,1)=8;
        refFaceToEdge(3,2)=7; refFaceToEdge(3,3)=11;
        refFaceToEdge(4,0)=0; refFaceToEdge(4,1)=1;
        refFaceToEdge(4,2)=2; refFaceToEdge(4,3)=3;
        refFaceToEdge(5,0)=4; refFaceToEdge(5,1)=5;
        refFaceToEdge(5,2)=6; refFaceToEdge(5,3)=7;
        

// *********************************** GENERATE MESH ************************************

  if (MyPID == 0) {
    std::cout << "Generating mesh ... \n\n";
  }

   // Generate mesh with Pamgen

    int dim=3;
    long long maxInt = 9223372036854775807LL;
    Create_Pamgen_Mesh(meshInput.c_str(), dim, rank, numProcs, maxInt);

   // Get local mesh size info
    char title[100];
    long long numDim;
    long long numNodes;
    long long numElems;
    long long numElemBlk;
    long long numNodeSets;
    long long numSideSets;
    int id = 0;

    im_ex_get_init_l(id, title, &numDim, &numNodes,
                                &numElems, &numElemBlk, &numNodeSets,
                                &numSideSets);

   // Get global mesh size info
    long long numNodesGlobal;
    long long numElemsGlobal;
    long long numElemBlkGlobal;
    long long numNodeSetsGlobal;
    long long numSideSetsGlobal;

    im_ne_get_init_global_l(id, &numNodesGlobal, &numElemsGlobal,
                         &numElemBlkGlobal, &numNodeSetsGlobal,
                         &numSideSetsGlobal);

    long long * block_ids = new long long [numElemBlk];
    error += im_ex_get_elem_blk_ids_l(id, block_ids);


    long long  *nodes_per_element   = new long long [numElemBlk];
    long long  *element_attributes  = new long long [numElemBlk];
    long long  *elements            = new long long [numElemBlk];
    char      **element_types       = new char * [numElemBlk];
    long long **elmt_node_linkage   = new long long * [numElemBlk];


    for(long long i = 0; i < numElemBlk; i ++){
      element_types[i] = new char [MAX_STR_LENGTH + 1];
      error += im_ex_get_elem_block_l(id,
                                      block_ids[i],
                                      element_types[i],
                                      (long long*)&(elements[i]),
                                      (long long*)&(nodes_per_element[i]),
                                      (long long*)&(element_attributes[i]));
    }

    /*connectivity*/
    for(long long b = 0; b < numElemBlk; b++){
      elmt_node_linkage[b] =  new long long [nodes_per_element[b]*elements[b]];
      error += im_ex_get_elem_conn_l(id,block_ids[b],elmt_node_linkage[b]);
    }

   // Get mu value for each block of elements from parameter list
    double  *mu = new double [numElemBlk];
    for(int b = 0; b < numElemBlk; b++){
       stringstream muBlock;
       muBlock.clear();
       muBlock << "mu" << b;
       mu[b] = inputList.get(muBlock.str(),1.0);
    }

  // Get node-element connectivity and set element mu value
    int telct = 0;
    FieldContainer<int> elemToNode(numElems,numNodesPerElem);
    FieldContainer<double> muVal(numElems);
    for(long long b = 0; b < numElemBlk; b++){
      for(long long el = 0; el < elements[b]; el++){
        for (int j=0; j<numNodesPerElem; j++) {
          elemToNode(telct,j) = elmt_node_linkage[b][el*numNodesPerElem + j]-1;
        }
        muVal(telct) = mu[b];
        telct ++;
      }
    }

   // Read node coordinates and place in field container
    FieldContainer<double> nodeCoord(numNodes,dim);
    double * nodeCoordx = new double [numNodes];
    double * nodeCoordy = new double [numNodes];
    double * nodeCoordz = new double [numNodes];
    im_ex_get_coord_l(id,nodeCoordx,nodeCoordy,nodeCoordz);
    for (int i=0; i<numNodes; i++) {
      nodeCoord(i,0)=nodeCoordx[i];
      nodeCoord(i,1)=nodeCoordy[i];
      nodeCoord(i,2)=nodeCoordz[i];
    }

    /*parallel info*/
    long long num_internal_nodes;
    long long num_border_nodes;
    long long num_external_nodes;
    long long num_internal_elems;
    long long num_border_elems;
    long long num_node_comm_maps;
    long long num_elem_comm_maps;
    im_ne_get_loadbal_param_l( id,
                               &num_internal_nodes,
                               &num_border_nodes,
                               &num_external_nodes,
                               &num_internal_elems,
                               &num_border_elems,
                               &num_node_comm_maps,
                               &num_elem_comm_maps,
                               0/*unused*/ );

    if(num_node_comm_maps > 0){
      node_comm_proc_ids   = new long long  [num_node_comm_maps];
      node_cmap_node_cnts  = new long long  [num_node_comm_maps];
      node_cmap_ids        = new long long  [num_node_comm_maps];
      comm_node_ids        = new long long* [num_node_comm_maps];
      comm_node_proc_ids   = new long long* [num_node_comm_maps];

      long long *  elem_cmap_ids        = new long long [num_elem_comm_maps];
      long long *  elem_cmap_elem_cnts  = new long long [num_elem_comm_maps];
     if ( im_ne_get_cmap_params_l( id,
                                  node_cmap_ids,
                                  (long long*)node_cmap_node_cnts,
                                  elem_cmap_ids,
                                  (long long*)elem_cmap_elem_cnts,
                                  0/*not used proc_id*/ ) < 0 )++error;

      for(long long j = 0; j < num_node_comm_maps; j++) {
        comm_node_ids[j]       = new long long [node_cmap_node_cnts[j]];
        comm_node_proc_ids[j]  = new long long [node_cmap_node_cnts[j]];
        if ( im_ne_get_node_cmap_l( id,
                                  node_cmap_ids[j],
                                  comm_node_ids[j],
                                  comm_node_proc_ids[j],
                                  0/*not used proc_id*/ ) < 0 )++error;
        node_comm_proc_ids[j] = comm_node_proc_ids[j][0];
      }
    }



    //Calculate global node ids
    long long * globalNodeIds = new long long[numNodes];
    bool * nodeIsOwned = new bool[numNodes];

    calc_global_node_ids(globalNodeIds,
                         nodeIsOwned,
                         numNodes,
                         num_node_comm_maps,
                         node_cmap_node_cnts,
                         node_comm_proc_ids,
                         comm_node_ids,
                         rank);

   // Count owned nodes
    int ownedNodes=0;
    for(int i=0;i<numNodes;i++)
      if(nodeIsOwned[i]) ownedNodes++;

   // Build a list of the OWNED global ids...
    int *ownedGIDs=new int [ownedNodes];
    int oidx=0;
    for(int i=0;i<numNodes;i++)
      if(nodeIsOwned[i]){
        ownedGIDs[oidx]=(int)globalNodeIds[i];
        oidx++;
      }

    FieldContainer<int> elemToEdge(numElems,numEdgesPerElem);
    FieldContainer<int> elemToFace(numElems,numFacesPerElem);

   // Calculate edge and face ids
    int elct = 0;
    for(long long b = 0; b < numElemBlk; b++){
      if(nodes_per_element[b] == 4){
      }
      else if (nodes_per_element[b] == 8){
        //loop over all elements and push their edges onto a set if they are not there already
        for(long long el = 0; el < elements[b]; el++){
          std::set< topo_entity *, fecomp > ::iterator fit;
          for (int i=0; i < numEdgesPerElem; i++){
            topo_entity * teof = new topo_entity;
            for(int j = 0; j < numNodesPerEdge;j++){
              teof->add_node(elmt_node_linkage[b][el*numNodesPerElem + refEdgeToNode(i,j)],globalNodeIds);
            }
            teof->sort();
            fit = edge_set.find(teof);
            if(fit == edge_set.end()){
              teof->local_id = edge_vector.size();
              edge_set.insert(teof);
              elemToEdge(elct,i)= edge_vector.size();
              edge_vector.push_back(teof);
            }
            else{
              elemToEdge(elct,i) = (*fit)->local_id;
              delete teof;
            }
          }
          for (int i=0; i < numFacesPerElem; i++){
            topo_entity * teof = new topo_entity;
            for(int j = 0; j < numNodesPerFace;j++){
              teof->add_node(elmt_node_linkage[b][el*numNodesPerElem + refFaceToNode(i,j)],globalNodeIds);
            }
            teof->sort();
            fit = face_set.find(teof);
            if(fit == face_set.end()){
              teof->local_id = face_vector.size();
              face_set.insert(teof);
              elemToFace(elct,i)= face_vector.size();
              face_vector.push_back(teof);
            }
            else{
              elemToFace(elct,i) = (*fit)->local_id;
              delete teof;
            }
          }
          elct ++;
        }
      }
    }

   // Edge to Node connectivity
    FieldContainer<int> edgeToNode(edge_vector.size(), numNodesPerEdge);
    for(unsigned ect = 0; ect != edge_vector.size(); ect++){
      std::list<long long>::iterator elit;
      int nct = 0;
      for(elit  = edge_vector[ect]->local_node_ids.begin();
          elit != edge_vector[ect]->local_node_ids.end();
          elit ++){
        edgeToNode(ect,nct) = *elit-1;
        nct++;
      }
    }

   // Face to Node connectivity
    FieldContainer<int> faceToNode(face_vector.size(), numNodesPerFace);
    for(unsigned fct = 0; fct != face_vector.size(); fct++){
      std::list<long long>::iterator flit;
      int nct = 0;
      for(flit  = face_vector[fct]->local_node_ids.begin();
          flit != face_vector[fct]->local_node_ids.end();
          flit ++){
        faceToNode(fct,nct) = *flit-1;
        nct++;
      }
    }

  // Face to Edge connectivity
    FieldContainer<int> faceToEdge(face_vector.size(), numEdgesPerFace);
    FieldContainer<bool> faceDone(face_vector.size());
    for (int ielem = 0; ielem < numElems; ielem++){
       for (int iface = 0; iface < numFacesPerElem; iface++){
         if (!faceDone(elemToFace(ielem,iface))){
           for (int iedge = 0; iedge < numEdgesPerFace; iedge++){
              faceToEdge(elemToFace(ielem,iface),iedge) =
                           elemToEdge(ielem,refFaceToEdge(iface,iedge));
              faceDone(elemToFace(ielem,iface))=1;
           }
         }
       }
    }

    int numEdges = edge_vector.size();
    int numFaces = face_vector.size();

   // Calculate global edge and face numbering
    std::string doing_type;
    doing_type = "EDGES";
    calc_global_ids(edge_vector,
               comm_node_ids,
               node_comm_proc_ids,
               node_cmap_node_cnts,
               num_node_comm_maps,
               rank,
               doing_type);
     

    doing_type = "FACES";
    calc_global_ids(face_vector,
               comm_node_ids,
               node_comm_proc_ids,
               node_cmap_node_cnts,
               num_node_comm_maps,
               rank,
               doing_type);

  // Build list of owned global edge ids
    long long * globalEdgeIds = new long long[numEdges];
    bool * edgeIsOwned = new bool[numEdges];
    int numOwnedEdges=0;
    for (int i=0; i<numEdges; i++) {
        edgeIsOwned[i] = edge_vector[i]->owned;
        globalEdgeIds[i] = edge_vector[i]->global_id;
        if (edgeIsOwned[i]){
           numOwnedEdges++;
        }
     }
    int * ownedEdgeIds = new int[numOwnedEdges];
    int nedge=0;
    for (int i=0; i<numEdges; i++) {
        if (edgeIsOwned[i]){
           ownedEdgeIds[nedge]=(int)globalEdgeIds[i];
           nedge++;
        }
     }

  // Build list of owned global face ids
    long long * globalFaceIds = new long long[numFaces];
    bool * faceIsOwned = new bool[numFaces];
    int numOwnedFaces=0;
    for (int i=0; i<numFaces; i++) {
        faceIsOwned[i] = face_vector[i]->owned;
        globalFaceIds[i] = face_vector[i]->global_id;
        if (faceIsOwned[i]){
           numOwnedFaces++;
        }
     }
    int * ownedFaceIds = new int[numOwnedFaces];
    int nface=0;
    for (int i=0; i<numFaces; i++) {
        if (faceIsOwned[i]){
           ownedFaceIds[nface]=(int)globalFaceIds[i];
           nface++;
        }
     }

  // Calculate number of global edges and faces
    int numEdgesGlobal;
    int numFacesGlobal;
#ifdef HAVE_MPI
    Comm.SumAll(&numOwnedEdges,&numEdgesGlobal,1);
    Comm.SumAll(&numOwnedFaces,&numFacesGlobal,1);
#else
    numEdgesGlobal = numEdges;
    numFacesGlobal = numFaces;
#endif

   // Define global epetra maps
    Epetra_Map globalMapG(-1,ownedNodes,ownedGIDs,0,Comm);
    Epetra_Map globalMapC(-1,numOwnedEdges,ownedEdgeIds,0,Comm);


 // Print mesh size information
  if (MyPID == 0) {
    std::cout << " Number of Elements: " << numElemsGlobal << " \n";
    std::cout << "    Number of Nodes: " << numNodesGlobal << " \n";
    std::cout << "    Number of Edges: " << numEdgesGlobal << " \n";
    std::cout << "    Number of Faces: " << numFacesGlobal << " \n\n";
  }


#ifdef DUMP_DATAE
  // Output element to face connectivity
   std::stringstream e2nfname;
      e2nfname << "elem2node";
      e2nfname << MyPID << ".dat";
   std::stringstream e2efname;
      e2efname << "elem2edge";
      e2efname << MyPID << ".dat";

    ofstream el2eout(e2efname.str().c_str());
    ofstream el2nout(e2nfname.str().c_str());
    for (int i=0; i<numElems; i++) {
      for (int m=0; m<numNodesPerElem; m++) {
        el2nout << globalNodeIds[elemToNode(i,m)] << "  ";
      }
      el2nout << "\n";
      for (int n=0; n<numEdgesPerElem; n++) {
        el2eout << globalEdgeIds[elemToEdge(i,n)] << "  ";
      }
      el2eout << "\n";
    }
    el2nout.close();
    el2eout.close();

   // Output edge to node connectivity
   std::stringstream ed2nfname;
      ed2nfname << "edge2node";
      ed2nfname << MyPID << ".dat";
    ofstream e2nout(ed2nfname.str().c_str());
    for (int j=0; j<numEdges; j++) {
       if (edgeIsOwned[j]){
         e2nout << globalNodeIds[edgeToNode(j,0)] << "  ";
         e2nout << globalNodeIds[edgeToNode(j,1)] << "\n";
       }
    }
    e2nout.close();
#endif

   // Container indicating whether a face is on the boundary (1-yes 0-no)
    FieldContainer<int> edgeOnBoundary(numEdges);
    FieldContainer<int> faceOnBoundary(numFaces);

  // Get boundary (side set) information
    long long * sideSetIds = new long long [numSideSets];
    long long numSidesInSet;
    long long numDFinSet;
    im_ex_get_side_set_ids_l(id,sideSetIds);
    for (int i=0; i<numSideSets; i++) {
        im_ex_get_side_set_param_l(id,sideSetIds[i],&numSidesInSet,&numDFinSet);
        if (numSidesInSet > 0){
          long long * sideSetElemList = new long long [numSidesInSet];
          long long * sideSetSideList = new long long [numSidesInSet];
          im_ex_get_side_set_l(id,sideSetIds[i],sideSetElemList,sideSetSideList);
          for (int j=0; j<numSidesInSet; j++) {
             int iface = sideSetSideList[j]-1;
             faceOnBoundary(elemToFace(sideSetElemList[j]-1,iface))=1;
             edgeOnBoundary(faceToEdge(elemToFace(sideSetElemList[j]-1,iface),0))=1;
             edgeOnBoundary(faceToEdge(elemToFace(sideSetElemList[j]-1,iface),1))=1;
             edgeOnBoundary(faceToEdge(elemToFace(sideSetElemList[j]-1,iface),2))=1;
             edgeOnBoundary(faceToEdge(elemToFace(sideSetElemList[j]-1,iface),3))=1;
          }
          delete [] sideSetElemList;
          delete [] sideSetSideList;
       }
    }

    delete [] sideSetIds;


   // Container indicating whether a node is on the boundary (1-yes 0-no)
    FieldContainer<int> nodeOnBoundary(numNodes);
     int numEdgeOnBndy=0;
     for (int i=0; i<numEdges; i++) {
        if (edgeOnBoundary(i)){
           nodeOnBoundary(edgeToNode(i,0))=1;
           nodeOnBoundary(edgeToNode(i,1))=1;
           numEdgeOnBndy++;
        }
     }   


#ifdef RANDOM_GRID
/***********************************************************************************/
/* This block of code will create a randomly perturbed mesh by jiggling the node
   coordinates up to 1/4 of an edge length away from their initial location.
         !!!NOTE: THIS WILL ONLY WORK CORRECTLY ON A SINGLE PROCESSOR!!!           */
/***********************************************************************************/
     // Side length assuming an initially regular grid
         double hx = nodeCoord(elemToNode(0,1),0)-nodeCoord(elemToNode(0,0),0);
         double hy = nodeCoord(elemToNode(0,3),1)-nodeCoord(elemToNode(0,0),1);
         double hz = nodeCoord(elemToNode(0,4),2)-nodeCoord(elemToNode(0,0),2);

     // Loop over nodes
       for (int inode = 0; inode < numNodes; inode++){
         if (!nodeOnBoundary(inode) & nodeIsOwned[inode]) {
           // random numbers between -1.0 and 1.0
            double rx = 2.0 * (double)rand()/RAND_MAX - 1.0;
            double ry = 2.0 * (double)rand()/RAND_MAX - 1.0;
            double rz = 2.0 * (double)rand()/RAND_MAX - 1.0;
           // limit variation to 1/4 edge length
            nodeCoord(inode,0) = nodeCoord(inode,0) + 0.125 * hx * rx;
            nodeCoord(inode,1) = nodeCoord(inode,1) + 0.125 * hy * ry;
            nodeCoord(inode,2) = nodeCoord(inode,2) + 0.125 * hz * rz;
            nodeCoordx[inode] = nodeCoord(inode,0);
            nodeCoordy[inode] = nodeCoord(inode,1);
            nodeCoordz[inode] = nodeCoord(inode,2);
         }
       }
#endif

   // Build the coordinate vectors for ML solver (including owned nodes only)
    Epetra_Vector Nx(globalMapG), Ny(globalMapG),Nz(globalMapG);
    for(int i=0,nlid=0;i<numNodes;i++)
      if(nodeIsOwned[i]) {
	Nx[nlid]=nodeCoordx[i];
	Ny[nlid]=nodeCoordy[i];
	Nz[nlid]=nodeCoordz[i];
	nlid++;
      }


// **************************** INCIDENCE MATRIX **************************************

   // Node to edge incidence matrix
  if (MyPID == 0) {
    std::cout << "Building incidence matrix ... \n\n";
  }

    Epetra_FECrsMatrix DGrad(Copy, globalMapC, 2);

    // Grab edge coordinates (for dumping to disk)
    Epetra_Vector EDGE_X(globalMapC);
    Epetra_Vector EDGE_Y(globalMapC);
    Epetra_Vector EDGE_Z(globalMapC);

    double vals[2];
    vals[0]=-1.0; vals[1]=1.0;
    for (int j=0, elid=0; j<numEdges; j++){
      if (edgeIsOwned[j]){
        int rowNum = globalEdgeIds[j];
        int colNum[2];
        colNum[0] = globalNodeIds[edgeToNode(j,0)];
        colNum[1] = globalNodeIds[edgeToNode(j,1)];
        DGrad.InsertGlobalValues(1, &rowNum, 2, colNum, vals);
	EDGE_X[elid] = (nodeCoordx[edgeToNode(j,0)] + nodeCoordx[edgeToNode(j,1)])/2.0;
	EDGE_Y[elid] = (nodeCoordy[edgeToNode(j,0)] + nodeCoordy[edgeToNode(j,1)])/2.0;
	EDGE_Z[elid] = (nodeCoordz[edgeToNode(j,0)] + nodeCoordz[edgeToNode(j,1)])/2.0;
	elid++;
      }
    }





// ************************************ CUBATURE **************************************

   // Get numerical integration points and weights for cell
  if (MyPID == 0) {
    std::cout << "Getting cubature ... \n\n";
  }

    DefaultCubatureFactory<double>  cubFactory;                                   
    int cubDegree = 2;
    Teuchos::RCP<Cubature<double> > hexCub = cubFactory.create(hex_8, cubDegree); 

    int cubDim       = hexCub->getDimension();
    int numCubPoints = hexCub->getNumPoints();

    FieldContainer<double> cubPoints(numCubPoints, cubDim);
    FieldContainer<double> cubWeights(numCubPoints);

    hexCub->getCubature(cubPoints, cubWeights);

  
   // Get numerical integration points and weights for hexahedron face
    //             (needed for rhs boundary term)

    // Define topology of the face parametrization domain as [-1,1]x[-1,1]
    CellTopology paramQuadFace(shards::getCellTopologyData<shards::Quadrilateral<4> >() );

    // Define cubature 
    DefaultCubatureFactory<double>  cubFactoryFace;
    Teuchos::RCP<Cubature<double> > hexFaceCubature = cubFactoryFace.create(paramQuadFace, 3);
    int cubFaceDim    = hexFaceCubature -> getDimension();
    int numFacePoints = hexFaceCubature -> getNumPoints();

    // Define storage for cubature points and weights on [-1,1]x[-1,1]
    FieldContainer<double> paramGaussWeights(numFacePoints);
    FieldContainer<double> paramGaussPoints(numFacePoints,cubFaceDim);

    // Define storage for cubature points on workset faces
    hexFaceCubature -> getCubature(paramGaussPoints, paramGaussWeights);

  // Get numerical integration points and weights for hexahedron edge
     //         (needed for inhomogeneous boundary terms)

    // Define topology of the edge parametrization domain as [-1,1]
    CellTopology paramEdge(shards::getCellTopologyData<shards::Line<2> >() );

    // Define cubature
    DefaultCubatureFactory<double>  cubFactoryEdge;
    Teuchos::RCP<Cubature<double> > hexEdgeCubature = cubFactoryEdge.create(paramEdge, 3);
    int cubEdgeDim    = hexEdgeCubature -> getDimension();
    int numEdgePoints = hexEdgeCubature -> getNumPoints();

    // Define storage for cubature points and weights on [-1,1]
    FieldContainer<double> paramEdgeWeights(numEdgePoints);
    FieldContainer<double> paramEdgePoints(numEdgePoints,cubEdgeDim);

    // Define storage for cubature points on workset faces
    hexEdgeCubature -> getCubature(paramEdgePoints, paramEdgeWeights);

// ************************************** BASIS ***************************************

   // Define basis 
  if (MyPID == 0) {
    std::cout << "Getting basis ... \n\n";
  }
    Basis_HCURL_HEX_I1_FEM<double, FieldContainer<double> > hexHCurlBasis;
    Basis_HGRAD_HEX_C1_FEM<double, FieldContainer<double> > hexHGradBasis;

    int numFieldsC = hexHCurlBasis.getCardinality();
    int numFieldsG = hexHGradBasis.getCardinality();

  // Evaluate basis at cubature points
     FieldContainer<double> hexGVals(numFieldsG, numCubPoints); 
     FieldContainer<double> hexCVals(numFieldsC, numCubPoints, spaceDim); 
     FieldContainer<double> hexCurls(numFieldsC, numCubPoints, spaceDim); 
     FieldContainer<double> worksetCVals(numFieldsC, numFacePoints, spaceDim); 

     hexHCurlBasis.getValues(hexCVals, cubPoints, OPERATOR_VALUE);
     hexHCurlBasis.getValues(hexCurls, cubPoints, OPERATOR_CURL);
     hexHGradBasis.getValues(hexGVals, cubPoints, OPERATOR_VALUE);

// *********************** INHOMOGENEOUS BC ***********************

    typedef CellTools<double>  CellTools;
    typedef FunctionSpaceTools fst;

    FieldContainer<double> bndyEdgeVal(numEdgeOnBndy);
    FieldContainer<int> bndyEdgeToEdge(numEdges);
    FieldContainer<bool> bndyEdgeDone(numEdges);
    FieldContainer<double> refEdgePoints(numEdgePoints,spaceDim);
    FieldContainer<double> bndyEdgePoints(1,numEdgePoints,spaceDim);
    FieldContainer<double> bndyEdgeJacobians(1,numEdgePoints,spaceDim,spaceDim);
    FieldContainer<double> edgeTan(1,numEdgePoints,spaceDim);
    FieldContainer<double> uDotTangent(numEdgePoints);
    FieldContainer<double> uEdge(numEdgePoints,spaceDim);
    FieldContainer<double> nodes(1, numNodesPerElem, spaceDim);

    int ibedge=0;
    // Evaluate tangent at edge quadrature points
    for (int ielem=0; ielem<numElems; ielem++) {
       for (int inode=0; inode<numNodesPerElem; inode++) {
         nodes(0,inode,0) = nodeCoord(elemToNode(ielem,inode),0);
         nodes(0,inode,1) = nodeCoord(elemToNode(ielem,inode),1);
         nodes(0,inode,2) = nodeCoord(elemToNode(ielem,inode),2);
       }
       for (int iedge=0; iedge<numEdgesPerElem; iedge++){
          if(edgeOnBoundary(elemToEdge(ielem,iedge)) && !bndyEdgeDone(elemToEdge(ielem,iedge))){

          // map evaluation points from reference edge to reference cell
             CellTools::mapToReferenceSubcell(refEdgePoints,
                                   paramEdgePoints,
                                   1, iedge, hex_8);

          // calculate Jacobian
             CellTools::setJacobian(bndyEdgeJacobians, refEdgePoints,
                         nodes, hex_8);

          // map evaluation points from reference cell to physical cell
             CellTools::mapToPhysicalFrame(bndyEdgePoints,
                                refEdgePoints,
                                nodes, hex_8);

          // Compute edge tangents
             CellTools::getPhysicalEdgeTangents(edgeTan,
                                              bndyEdgeJacobians,
                                              iedge, hex_8);

          // evaluate exact solution at edge center and dot with normal
           for(int nPt = 0; nPt < numEdgePoints; nPt++){

             double x = bndyEdgePoints(0, nPt, 0);
             double y = bndyEdgePoints(0, nPt, 1);
             double z = bndyEdgePoints(0, nPt, 2);

             evalu(uEdge(nPt,0), uEdge(nPt,1), uEdge(nPt,2), x, y, z);
             uDotTangent(nPt)=(uEdge(nPt,0)*edgeTan(0,nPt,0)+uEdge(nPt,1)*edgeTan(0,nPt,1)+uEdge(nPt,2)*edgeTan(0,nPt,2));
           }

          // integrate
           for(int nPt = 0; nPt < numEdgePoints; nPt++){
             bndyEdgeVal(ibedge)=bndyEdgeVal(ibedge)+uDotTangent(nPt)*paramEdgeWeights(nPt);
           }

           bndyEdgeToEdge(elemToEdge(ielem,iedge))=ibedge;
           ibedge++;
           bndyEdgeDone(elemToEdge(ielem,iedge))=1;
         }
       }
    }



// ******** LOOP OVER ELEMENTS TO CREATE LOCAL MASS and STIFFNESS MATRICES *************


  if (MyPID == 0) {
    std::cout << "Building mass and stiffness matrices ... \n\n";
  }

 // Settings and data structures for mass and stiffness matrices
    typedef ArrayTools art;
    int numCells = 1; 

   // Containers for nodes and edge signs 
    FieldContainer<double> hexNodes(numCells, numNodesPerElem, spaceDim);
    FieldContainer<double> hexEdgeSigns(numCells, numFieldsC);
   // Containers for Jacobian
    FieldContainer<double> hexJacobian(numCells, numCubPoints, spaceDim, spaceDim);
    FieldContainer<double> hexJacobInv(numCells, numCubPoints, spaceDim, spaceDim);
    FieldContainer<double> hexJacobDet(numCells, numCubPoints);
   // Containers for element HGRAD mass matrix
    FieldContainer<double> massMatrixG(numCells, numFieldsG, numFieldsG);
    FieldContainer<double> weightedMeasure(numCells, numCubPoints);
    FieldContainer<double> weightedMeasureMuInv(numCells, numCubPoints);
    FieldContainer<double> hexGValsTransformed(numCells, numFieldsG, numCubPoints);
    FieldContainer<double> hexGValsTransformedWeighted(numCells, numFieldsG, numCubPoints);
   // Containers for element HCURL mass matrix
    FieldContainer<double> massMatrixC(numCells, numFieldsC, numFieldsC);
    FieldContainer<double> hexCValsTransformed(numCells, numFieldsC, numCubPoints, spaceDim);
    FieldContainer<double> hexCValsTransformedWeighted(numCells, numFieldsC, numCubPoints, spaceDim);
   // Containers for element HCURL stiffness matrix
    FieldContainer<double> stiffMatrixC(numCells, numFieldsC, numFieldsC);
    FieldContainer<double> weightedMeasureMu(numCells, numCubPoints);    
    FieldContainer<double> hexCurlsTransformed(numCells, numFieldsC, numCubPoints, spaceDim);
    FieldContainer<double> hexCurlsTransformedWeighted(numCells, numFieldsC, numCubPoints, spaceDim);
   // Containers for right hand side vectors
    FieldContainer<double> rhsDatag(numCells, numCubPoints, cubDim);
    FieldContainer<double> rhsDatah(numCells, numCubPoints, cubDim);
    FieldContainer<double> gC(numCells, numFieldsC);
    FieldContainer<double> hC(numCells, numFieldsC);
    FieldContainer<double> hCBoundary(numCells, numFieldsC);
    FieldContainer<double> refGaussPoints(numFacePoints,spaceDim);
    FieldContainer<double> worksetGaussPoints(numCells,numFacePoints,spaceDim);
    FieldContainer<double> worksetJacobians(numCells, numFacePoints, spaceDim, spaceDim);
    FieldContainer<double> worksetJacobInv(numCells, numFacePoints, spaceDim, spaceDim);
    FieldContainer<double> worksetFaceN(numCells, numFacePoints, spaceDim);
    FieldContainer<double> worksetVFieldVals(numCells, numFacePoints, spaceDim);
    FieldContainer<double> worksetCValsTransformed(numCells, numFieldsC, numFacePoints, spaceDim);
    FieldContainer<double> divuFace(numCells, numFacePoints);
    FieldContainer<double> worksetFieldDotNormal(numCells, numFieldsC, numFacePoints);

   // Container for cubature points in physical space
    FieldContainer<double> physCubPoints(numCells,numCubPoints, cubDim);

    
   // Global arrays in Epetra format
    Epetra_FECrsMatrix MassG(Copy, globalMapG, numFieldsG);
    Epetra_FECrsMatrix MassC(Copy, globalMapC, numFieldsC);
    Epetra_FECrsMatrix StiffC(Copy, globalMapC, numFieldsC);
    Epetra_FEVector rhsC(globalMapC);

#ifdef DUMP_DATAE
    std::stringstream eSignfname;
      eSignfname << "edgeSigns";
      eSignfname << MyPID << ".dat";
    ofstream fSignsout(eSignfname.str().c_str());
#endif

 // *** Element loop ***
    for (int k=0; k<numElems; k++) {

     // Physical cell coordinates
      for (int i=0; i<numNodesPerElem; i++) {
         hexNodes(0,i,0) = nodeCoord(elemToNode(k,i),0);
         hexNodes(0,i,1) = nodeCoord(elemToNode(k,i),1);
         hexNodes(0,i,2) = nodeCoord(elemToNode(k,i),2);
      }

     // Edge signs
      for (int j=0; j<numEdgesPerElem; j++) {
          if (elemToNode(k,refEdgeToNode(j,0))==edgeToNode(elemToEdge(k,j),0) &&
              elemToNode(k,refEdgeToNode(j,1))==edgeToNode(elemToEdge(k,j),1))
              hexEdgeSigns(0,j) = 1.0;
          else 
              hexEdgeSigns(0,j) = -1.0;
       }
       

      // modify signs for edges whose signs were defined on another processor
       if (!faceIsOwned[elemToFace(k,0)]) {
            hexEdgeSigns(0,0)=-1.0*hexEdgeSigns(0,0);
            hexEdgeSigns(0,4)=-1.0*hexEdgeSigns(0,4);
        }
       if (!faceIsOwned[elemToFace(k,1)]) {
            hexEdgeSigns(0,1)=-1.0*hexEdgeSigns(0,1);
            hexEdgeSigns(0,5)=-1.0*hexEdgeSigns(0,5);
        }
       if (!faceIsOwned[elemToFace(k,2)]) {
            hexEdgeSigns(0,2)=-1.0*hexEdgeSigns(0,2);
            hexEdgeSigns(0,6)=-1.0*hexEdgeSigns(0,6);
        }
       if (!faceIsOwned[elemToFace(k,3)]) {
            hexEdgeSigns(0,3)=-1.0*hexEdgeSigns(0,3);
            hexEdgeSigns(0,7)=-1.0*hexEdgeSigns(0,7);
        }


#ifdef DUMP_DATAE
     for (int j=0; j<numEdgesPerElem; j++) {
        fSignsout << hexEdgeSigns(0,j) << "  ";
      } 
      fSignsout << "\n";
#endif

       // Compute cell Jacobians, their inverses and their determinants
       CellTools::setJacobian(hexJacobian, cubPoints, hexNodes, hex_8);
       CellTools::setJacobianInv(hexJacobInv, hexJacobian );
       CellTools::setJacobianDet(hexJacobDet, hexJacobian );

// ************************** Compute element HGrad mass matrices *******************************
  
     // transform to physical coordinates 
      fst::HGRADtransformVALUE<double>(hexGValsTransformed, hexGVals);
      
     // compute weighted measure
      fst::computeCellMeasure<double>(weightedMeasure, hexJacobDet, cubWeights);

      // combine mu value with weighted measure
      for (int nC = 0; nC < numCells; nC++){
        for (int nPt = 0; nPt < numCubPoints; nPt++){
          weightedMeasureMuInv(nC,nPt) = weightedMeasure(nC,nPt) / muVal(k);
        }
      }
      
     // multiply values with weighted measure
      fst::multiplyMeasure<double>(hexGValsTransformedWeighted,
                                   weightedMeasureMuInv, hexGValsTransformed);

     // integrate to compute element mass matrix
      fst::integrate<double>(massMatrixG,
                             hexGValsTransformed, hexGValsTransformedWeighted, COMP_CPP);

      // assemble into global matrix
      int err = 0;
      for (int row = 0; row < numFieldsG; row++){
        for (int col = 0; col < numFieldsG; col++){
            int rowIndex = globalNodeIds[elemToNode(k,row)];
            int colIndex = globalNodeIds[elemToNode(k,col)];
            double val = massMatrixG(0,row,col);
            MassG.InsertGlobalValues(1, &rowIndex, 1, &colIndex, &val);
         }
      }

// ************************** Compute element HCurl mass matrices *******************************

     // transform to physical coordinates 
      fst::HCURLtransformVALUE<double>(hexCValsTransformed, hexJacobInv, 
                                   hexCVals);

     // multiply by weighted measure
      fst::multiplyMeasure<double>(hexCValsTransformedWeighted,
                                   weightedMeasure, hexCValsTransformed);

     // integrate to compute element mass matrix
      fst::integrate<double>(massMatrixC,
                             hexCValsTransformed, hexCValsTransformedWeighted,
                             COMP_CPP);

     // apply edge signs
      fst::applyLeftFieldSigns<double>(massMatrixC, hexEdgeSigns);
      fst::applyRightFieldSigns<double>(massMatrixC, hexEdgeSigns);


     // assemble into global matrix
      err = 0;
      for (int row = 0; row < numFieldsC; row++){
        for (int col = 0; col < numFieldsC; col++){
            int rowIndex = globalEdgeIds[elemToEdge(k,row)];
            int colIndex = globalEdgeIds[elemToEdge(k,col)];
            double val = massMatrixC(0,row,col);
            MassC.InsertGlobalValues(1, &rowIndex, 1, &colIndex, &val);
         }
      }

// ************************ Compute element HCurl stiffness matrices *****************************

      // transform to physical coordinates 
      fst::HCURLtransformCURL<double>(hexCurlsTransformed, hexJacobian, hexJacobDet, 
                                      hexCurls);

      // combine mu value with weighted measure
      for (int nC = 0; nC < numCells; nC++){
        for (int nPt = 0; nPt < numCubPoints; nPt++){
          weightedMeasureMu(nC,nPt) = weightedMeasure(nC,nPt) * muVal(k);
          //weightedMeasureMu(nC,nPt) = weightedMeasure(nC,nPt);
         }
      }

     // multiply by weighted measure
      fst::multiplyMeasure<double>(hexCurlsTransformedWeighted,
                                   weightedMeasureMu, hexCurlsTransformed);

     // integrate to compute element stiffness matrix
      fst::integrate<double>(stiffMatrixC,
                             hexCurlsTransformed, hexCurlsTransformedWeighted,
                             COMP_CPP);

     // apply edge signs
      fst::applyLeftFieldSigns<double>(stiffMatrixC, hexEdgeSigns);
      fst::applyRightFieldSigns<double>(stiffMatrixC, hexEdgeSigns);

     // assemble into global matrix
      err = 0;
      for (int row = 0; row < numFieldsC; row++){
        for (int col = 0; col < numFieldsC; col++){
            int rowIndex = globalEdgeIds[elemToEdge(k,row)];
            int colIndex = globalEdgeIds[elemToEdge(k,col)];
            double val = stiffMatrixC(0,row,col);
            StiffC.InsertGlobalValues(1, &rowIndex, 1, &colIndex, &val);
         }
      }

// ******************************* Build right hand side ************************************

      // transform integration points to physical points
       CellTools::mapToPhysicalFrame(physCubPoints, cubPoints, hexNodes, hex_8);

      // evaluate right hand side functions at physical points
       for (int nPt = 0; nPt < numCubPoints; nPt++){

          double x = physCubPoints(0,nPt,0);
          double y = physCubPoints(0,nPt,1);
          double z = physCubPoints(0,nPt,2);
          double du1, du2, du3;

          evalCurlu(du1, du2, du3, x, y, z, muVal(k));
          rhsDatag(0,nPt,0) = du1;
          rhsDatag(0,nPt,1) = du2;
          rhsDatag(0,nPt,2) = du3;
         
          evalGradDivu(du1, du2, du3,  x, y, z, muVal(k));
          rhsDatah(0,nPt,0) = du1;
          rhsDatah(0,nPt,1) = du2;
          rhsDatah(0,nPt,2) = du3;
       }

     // integrate (g,curl w) term
      fst::integrate<double>(gC, rhsDatag, hexCurlsTransformedWeighted,
                             COMP_CPP);

     // integrate (h,div w) term
      fst::integrate<double>(hC, rhsDatah, hexCValsTransformedWeighted,
                             COMP_CPP);

     // apply signs
      fst::applyFieldSigns<double>(gC, hexEdgeSigns);
      fst::applyFieldSigns<double>(hC, hexEdgeSigns);

    // evaluate boundary term
      for (int i = 0; i < numFacesPerElem; i++){
        if (faceOnBoundary(elemToFace(k,i))){
           
         // map Gauss points on quad to reference face: paramGaussPoints -> refGaussPoints
            CellTools::mapToReferenceSubcell(refGaussPoints,
                                   paramGaussPoints,
                                   2, i, hex_8);

         // get basis values at points on reference cell
           hexHCurlBasis.getValues(worksetCVals, refGaussPoints, OPERATOR_VALUE);

         // compute Jacobians at Gauss pts. on reference face for all parent cells
           CellTools::setJacobian(worksetJacobians,
                         refGaussPoints,
                         hexNodes, hex_8);
           CellTools::setJacobianInv(worksetJacobInv, worksetJacobians );

         // transform to physical coordinates 
            fst::HCURLtransformVALUE<double>(worksetCValsTransformed, worksetJacobInv, 
                                   worksetCVals);

         // map Gauss points on quad from ref. face to face workset: refGaussPoints -> worksetGaussPoints
            CellTools::mapToPhysicalFrame(worksetGaussPoints,
                                refGaussPoints,
                                hexNodes, hex_8);

         // Compute face normals
            CellTools::getPhysicalFaceNormals(worksetFaceN,
                                              worksetJacobians,
                                              i, hex_8);

         // evaluate div u at face points
           for(int nPt = 0; nPt < numFacePoints; nPt++){

             double x = worksetGaussPoints(0, nPt, 0);
             double y = worksetGaussPoints(0, nPt, 1);
             double z = worksetGaussPoints(0, nPt, 2);

             divuFace(0,nPt)=evalDivu(x, y, z, muVal(k));
           }

          // compute the dot product and multiply by Gauss weights
           for (int nF = 0; nF < numFieldsC; nF++){
              for(int nPt = 0; nPt < numFacePoints; nPt++){
                 worksetFieldDotNormal(0,nF,nPt)=0.0;
                  for (int dim = 0; dim < spaceDim; dim++){
                      worksetFieldDotNormal(0,nF,nPt) += worksetCValsTransformed(0,nF,nPt,dim)
                                              * worksetFaceN(0,nPt,dim) * paramGaussWeights(nPt);
                  } //dim
              } //nPt
           } //nF

          // integrate 
          fst::integrate<double>(hCBoundary, divuFace, worksetFieldDotNormal,
                             COMP_CPP);

          // apply signs
           fst::applyFieldSigns<double>(hCBoundary, hexEdgeSigns);

          // add into hC term
            for (int nF = 0; nF < numFieldsC; nF++){
                hC(0,nF) = hC(0,nF) - hCBoundary(0,nF);
            }
          
        } // if faceOnBoundary
      } // numFaces


    // assemble into global vector
     for (int row = 0; row < numFieldsC; row++){
           int rowIndex = globalEdgeIds[elemToEdge(k,row)];
           double val = gC(0,row)-hC(0,row);
           rhsC.SumIntoGlobalValues(1, &rowIndex, &val);
     }

     
    } // *** end element loop ***

    // Assemble over multiple processors, if necessary
    MassG.GlobalAssemble();  MassG.FillComplete();
    MassC.GlobalAssemble();  MassC.FillComplete();
    StiffC.GlobalAssemble(); StiffC.FillComplete();
    rhsC.GlobalAssemble();

    DGrad.GlobalAssemble(globalMapG,globalMapC); DGrad.FillComplete(MassG.RowMap(),MassC.RowMap()); 

#ifdef DUMP_DATA
   // Matrices and Vector before adjusting due to BCs
   EpetraExt::RowMatrixToMatlabFile("mag_m1_0_matrix.dat",MassC);
   EpetraExt::RowMatrixToMatlabFile("mag_k1_0_matrix.dat",StiffC);
   EpetraExt::MultiVectorToMatrixMarketFile("mag_rhs1_clean.dat",rhsC,0,0,false);

    // Node Coordinates
    EpetraExt::VectorToMatrixMarketFile("coordx.dat",Nx,0,0,false);
    EpetraExt::VectorToMatrixMarketFile("coordy.dat",Ny,0,0,false);
    EpetraExt::VectorToMatrixMarketFile("coordz.dat",Nz,0,0,false);
    
    // Edge Coordinates
    EpetraExt::VectorToMatrixMarketFile("ecoordx.dat",EDGE_X);
    EpetraExt::VectorToMatrixMarketFile("ecoordy.dat",EDGE_Y);
    EpetraExt::VectorToMatrixMarketFile("ecoordz.dat",EDGE_Z);     
#endif

  
   // Build the inverse diagonal for MassG
   Epetra_Vector DiagG(MassG.RowMap());
   DiagG.PutScalar(1.0);
   MassG.Multiply(false,DiagG,DiagG);
   for(int i=0;i<DiagG.MyLength();i++) DiagG[i]=1.0/DiagG[i];
   Epetra_CrsMatrix MassGinv(Copy,MassG.RowMap(),MassG.RowMap(),1);
   for(int i=0;i<DiagG.MyLength();i++) {
     int CID=MassG.GCID(i);
     MassGinv.InsertGlobalValues(MassG.GRID(i),1,&(DiagG[i]),&CID);
   }
   MassGinv.FillComplete();
   
   // Zero out entries that correspond to boundary nodes
   for(int i=0;i<numNodes;i++) {
     if (nodeOnBoundary(i)){
      double val=0.0;
      int index = globalNodeIds[i];
      MassGinv.ReplaceGlobalValues(index,1,&val,&index);
     }
   }

   std::cout << "Adjust for boundary conditions... "<< "\n";

  // Count of boundary edges
    int numBCEdges=0;
    for (int i=0; i<numEdges; i++){
      if (edgeOnBoundary(i) && edgeIsOwned[i]){
        numBCEdges++;
      }
    }
    std::cout<< "numBCEdges " <<numBCEdges<<"\n";

  // Vector for use in applying BCs
   Epetra_MultiVector v(globalMapC,true);
   v.PutScalar(0.0);

   // Set v to boundary values on Dirichlet edges
    int * BCEdges = new int [numBCEdges];
    int indbc=0;
    int indOwned=0;
    for (int i=0; i<numEdges; i++){
      if (edgeIsOwned[i]){
        if (edgeOnBoundary(i)){
           BCEdges[indbc]=indOwned;
           indbc++;
           v[0][indOwned]=bndyEdgeVal(bndyEdgeToEdge(i));
        }
        indOwned++;
      }
    }

  // Get the full matrix
   ML_Epetra::ML_RefMaxwell_11_Operator MatrixC(StiffC,DGrad,MassGinv,MassC);

  // Apply it to v
   Epetra_MultiVector rhsDir(globalMapC,true);
   //rhsDir.PutScalar(0.0);
   MatrixC.Apply(v,rhsDir);

  // Update right-hand side
   rhsC.Update(-1.0,rhsDir,1.0);


   // Adjust rhs due to Dirichlet boundary conditions
    indbc=0;
    indOwned=0;
    for (int i=0; i<numEdges; i++){
      if (edgeIsOwned[i]){
        if (edgeOnBoundary(i)){
           indbc++;
           rhsC[0][indOwned]=bndyEdgeVal(bndyEdgeToEdge(i));
           rhsDir[0][indOwned]=0.0;
        }
        indOwned++;
      }
    }

   // Zero out rows and columns of stiffness matrix corresponding to Dirichlet edges
   //  and add one to diagonal.
    ML_Epetra::Apply_OAZToMatrix(BCEdges, numBCEdges, StiffC);
    std::cout<< "indbc " <<indbc<<"\n";

    delete [] BCEdges;


   // Solve!
   Teuchos::ParameterList MLList,dummy;
   double TotalErrorResidual=0, TotalErrorExactSol=0;   
   ML_Epetra::SetDefaultsRefMaxwell(MLList);
   Teuchos::ParameterList MLList2=MLList.get("refmaxwell: 11list",MLList);
   MLList2.set("aggregation: type","Uncoupled-MIS");
   MLList2.set("x-coordinates",&Nx[0]);
   MLList2.set("y-coordinates",&Ny[0]);
   MLList2.set("z-coordinates",&Nz[0]);   
   MLList2.set("ML output",10);
   MLList2.set("smoother: sweeps (level 0)",3);
   MLList2.set("smoother: sweeps",3);
   MLList2.set("smoother: type","Chebyshev");
   //   MLList2.set("eigen-analysis: max iters", 20);
   MLList2.set("eigen-analysis: type", "power-method");
   MLList2.get("edge matrix free: coarse",dummy);
   ML_Epetra::SetDefaults("SA",dummy,0,0,false);
   dummy.set("PDE equations",3);
   dummy.set("ML output",10);
   dummy.set("smoother: sweeps",3);
   dummy.set("smoother: type","Chebyshev");
   dummy.set("aggregation: type","Uncoupled");
   dummy.set("smoother: pre or post","both");
   dummy.set("max levels",10);
   dummy.set("coarse: type","Amesos-KLU");
   dummy.set("repartition: enable",1);
   dummy.set("repartition: min per proc",1000);
   dummy.set("repartition: max min ratio",1.4);
   dummy.set("repartition: Zoltan dimensions",3);
   dummy.set("x-coordinates",&Nx[0]);
   dummy.set("y-coordinates",&Ny[0]);
   dummy.set("z-coordinates",&Nz[0]);   
   MLList2.set("edge matrix free: coarse",dummy);
   
  if (MyPID == 0) {
   cout<<MLList2<<endl;
  }
   

   Epetra_FEVector xh(rhsC);

   MassC.SetLabel("M1");
   StiffC.SetLabel("K1");
   DGrad.SetLabel("D0");
   MassGinv.SetLabel("M0^{-1}");
   
   char probType[12] = "curl_lsfem";

   TestMultiLevelPreconditioner_CurlLSFEM(probType,MLList2,StiffC,
                                          DGrad,MassGinv,MassC,
                                          xh,rhsC,
                                          TotalErrorResidual, TotalErrorExactSol);

    // ********  Calculate Error in Solution ***************

     double L2err = 0.0;
     double HCurlerr = 0.0;
     double Linferr = 0.0;
     double L2errTot = 0.0;
     double HCurlerrTot = 0.0;
     double LinferrTot = 0.0;

#ifdef HAVE_MPI
   // Import solution onto current processor
     Epetra_Map  solnMap(numEdgesGlobal, numEdgesGlobal, 0, Comm);
     Epetra_Import  solnImporter(solnMap, globalMapC);
     Epetra_FEVector  uCoeff(solnMap);
     uCoeff.Import(xh, solnImporter, Insert);
#endif
     

   // Get cubature points and weights for error calc (may be different from previous)
     DefaultCubatureFactory<double>  cubFactoryErr;
     int cubDegErr = 3;
     Teuchos::RCP<Cubature<double> > hexCubErr = cubFactoryErr.create(hex_8, cubDegErr);
     int cubDimErr       = hexCubErr->getDimension();
     int numCubPointsErr = hexCubErr->getNumPoints();
     FieldContainer<double> cubPointsErr(numCubPointsErr, cubDimErr);
     FieldContainer<double> cubWeightsErr(numCubPointsErr);
     hexCubErr->getCubature(cubPointsErr, cubWeightsErr);
     FieldContainer<double> physCubPointsE(numCells,numCubPointsErr, cubDimErr);

   // Containers for Jacobian
     FieldContainer<double> hexJacobianE(numCells, numCubPointsErr, spaceDim, spaceDim);
     FieldContainer<double> hexJacobInvE(numCells, numCubPointsErr, spaceDim, spaceDim);
     FieldContainer<double> hexJacobDetE(numCells, numCubPointsErr);
     FieldContainer<double> weightedMeasureE(numCells, numCubPointsErr);

 // Evaluate basis values and curls at cubature points
     FieldContainer<double> uhCVals(numFieldsC, numCubPointsErr, spaceDim);
     FieldContainer<double> uhCValsTrans(numCells,numFieldsC, numCubPointsErr, spaceDim);
     FieldContainer<double> uhCurls(numFieldsC, numCubPointsErr, spaceDim);
     FieldContainer<double> uhCurlsTrans(numCells, numFieldsC, numCubPointsErr, spaceDim);
     hexHCurlBasis.getValues(uhCVals, cubPointsErr, OPERATOR_VALUE);
     hexHCurlBasis.getValues(uhCurls, cubPointsErr, OPERATOR_CURL);


   // Loop over elements
    for (int k=0; k<numElems; k++){

      double L2errElem = 0.0;
      double HCurlerrElem = 0.0;
      double uExact1, uExact2, uExact3;
      double curluExact1, curluExact2, curluExact3;

     // physical cell coordinates
      for (int i=0; i<numNodesPerElem; i++) {
         hexNodes(0,i,0) = nodeCoord(elemToNode(k,i),0);
         hexNodes(0,i,1) = nodeCoord(elemToNode(k,i),1);
         hexNodes(0,i,2) = nodeCoord(elemToNode(k,i),2);
      }
     // Edge signs
      for (int j=0; j<numEdgesPerElem; j++) {
          if (elemToNode(k,refEdgeToNode(j,0))==edgeToNode(elemToEdge(k,j),0) &&
              elemToNode(k,refEdgeToNode(j,1))==edgeToNode(elemToEdge(k,j),1))
              hexEdgeSigns(0,j) = 1.0;
          else 
              hexEdgeSigns(0,j) = -1.0;
      } 

      // modify signs for edges whose signs were defined on another processor
       if (!faceIsOwned[elemToFace(k,0)]) {
            hexEdgeSigns(0,0)=-1.0*hexEdgeSigns(0,0);
            hexEdgeSigns(0,4)=-1.0*hexEdgeSigns(0,4);
        }
       if (!faceIsOwned[elemToFace(k,1)]) {
            hexEdgeSigns(0,1)=-1.0*hexEdgeSigns(0,1);
            hexEdgeSigns(0,5)=-1.0*hexEdgeSigns(0,5);
        }
       if (!faceIsOwned[elemToFace(k,2)]) {
            hexEdgeSigns(0,2)=-1.0*hexEdgeSigns(0,2);
            hexEdgeSigns(0,6)=-1.0*hexEdgeSigns(0,6);
        }
       if (!faceIsOwned[elemToFace(k,3)]) {
            hexEdgeSigns(0,3)=-1.0*hexEdgeSigns(0,3);
            hexEdgeSigns(0,7)=-1.0*hexEdgeSigns(0,7);
        }

    // compute cell Jacobians, their inverses and their determinants
       CellTools::setJacobian(hexJacobianE, cubPointsErr, hexNodes, hex_8);
       CellTools::setJacobianInv(hexJacobInvE, hexJacobianE );
       CellTools::setJacobianDet(hexJacobDetE, hexJacobianE );

      // transform integration points to physical points
       CellTools::mapToPhysicalFrame(physCubPointsE, cubPointsErr, hexNodes, hex_8);

      // transform basis values to physical coordinates
       fst::HCURLtransformVALUE<double>(uhCValsTrans, hexJacobInvE, uhCVals);
       fst::HCURLtransformCURL<double>(uhCurlsTrans, hexJacobianE, hexJacobDetE, uhCurls);

      // compute weighted measure
       fst::computeCellMeasure<double>(weightedMeasureE, hexJacobDetE, cubWeightsErr);

     // loop over cubature points
       for (int nPt = 0; nPt < numCubPointsErr; nPt++){

         // get exact solution and curls
          double x = physCubPointsE(0,nPt,0);
          double y = physCubPointsE(0,nPt,1);
          double z = physCubPointsE(0,nPt,2);
          double mu = 1.0;
          evalu(uExact1, uExact2, uExact3, x, y, z);
          evalCurlu(curluExact1, curluExact2, curluExact3, x, y, z, mu);

         // calculate approximate solution and curls
          double uApprox1 = 0.0;
          double uApprox2 = 0.0;
          double uApprox3 = 0.0;
          double curluApprox1 = 0.0;
          double curluApprox2= 0.0;
          double curluApprox3 = 0.0;
          for (int i = 0; i < numFieldsC; i++){
             int rowIndex = globalEdgeIds[elemToEdge(k,i)];
#ifdef HAVE_MPI
             double uh1 = uCoeff.Values()[rowIndex];
#else
             double uh1 = xh.Values()[rowIndex];
#endif
             uApprox1 += uh1*uhCValsTrans(0,i,nPt,0)*hexEdgeSigns(0,i);
             uApprox2 += uh1*uhCValsTrans(0,i,nPt,1)*hexEdgeSigns(0,i);
             uApprox3 += uh1*uhCValsTrans(0,i,nPt,2)*hexEdgeSigns(0,i);
             curluApprox1 += uh1*uhCurlsTrans(0,i,nPt,0)*hexEdgeSigns(0,i);
             curluApprox2 += uh1*uhCurlsTrans(0,i,nPt,1)*hexEdgeSigns(0,i);
             curluApprox3 += uh1*uhCurlsTrans(0,i,nPt,2)*hexEdgeSigns(0,i);
          }

         // evaluate the error at cubature points
          Linferr = max(Linferr, abs(uExact1 - uApprox1));
          Linferr = max(Linferr, abs(uExact2 - uApprox2));
          Linferr = max(Linferr, abs(uExact3 - uApprox3));
          L2errElem+=(uExact1 - uApprox1)*(uExact1 - uApprox1)*weightedMeasureE(0,nPt);
          L2errElem+=(uExact2 - uApprox2)*(uExact2 - uApprox2)*weightedMeasureE(0,nPt);
          L2errElem+=(uExact3 - uApprox3)*(uExact3 - uApprox3)*weightedMeasureE(0,nPt);
          HCurlerrElem+=((curluExact1 - curluApprox1)*(curluExact1 - curluApprox1))
                     *weightedMeasureE(0,nPt);
          HCurlerrElem+=((curluExact2 - curluApprox2)*(curluExact2 - curluApprox2))
                     *weightedMeasureE(0,nPt);
          HCurlerrElem+=((curluExact3 - curluApprox3)*(curluExact3 - curluApprox3))
                     *weightedMeasureE(0,nPt);
        }

       L2err+=L2errElem;
       HCurlerr+=HCurlerrElem;
     }

#ifdef HAVE_MPI
   // sum over all processors
    Comm.SumAll(&L2err,&L2errTot,1);
    Comm.SumAll(&HCurlerr,&HCurlerrTot,1);
    Comm.MaxAll(&Linferr,&LinferrTot,1);
#else
    L2errTot = L2err;
    HCurlerrTot = HCurlerr;
    LinferrTot = Linferr;
#endif

  if (MyPID == 0) {
    std::cout << "\n" << "L2 Error:  " << sqrt(L2errTot) <<"\n";
    std::cout << "HCurl Error:  " << sqrt(HCurlerrTot) <<"\n";
    std::cout << "LInf Error:  " << LinferrTot <<"\n\n";
  }


#ifdef DUMP_DATAE
   fSignsout.close();
#endif
 // delete mesh
 Delete_Pamgen_Mesh();

 //clean up
  for(long long b = 0; b < numElemBlk; b++){
     delete [] elmt_node_linkage[b];
     delete [] element_types[b];
   }
   delete [] block_ids;
   delete [] nodes_per_element;
   delete [] element_attributes;
   delete [] element_types;
   delete [] elmt_node_linkage;
   delete [] elements;
   delete [] nodeCoordx;
   delete [] nodeCoordy;
   delete [] nodeCoordz;

   delete [] globalNodeIds;
   delete [] nodeIsOwned;
   delete [] globalEdgeIds;
   delete [] edgeIsOwned;
   delete [] globalFaceIds;
   delete [] faceIsOwned;
   if(num_node_comm_maps > 0){
      delete [] node_comm_proc_ids;
      delete [] node_cmap_node_cnts;
      delete [] node_cmap_ids;
      for(long long i=0;i<num_node_comm_maps;i++){
        delete [] comm_node_ids[i];
        delete [] comm_node_proc_ids[i];
      }

      delete [] comm_node_ids;
      delete [] comm_node_proc_ids;
   }

   return 0;
}


/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
// Multiplies Ax = y, where all non-zero entries of A are replaced with the value 1.0
int Multiply_Ones(const Epetra_CrsMatrix &A,const Epetra_Vector &x,Epetra_Vector &y){
  if(!A.Filled()) 
    EPETRA_CHK_ERR(-1); // Matrix must be filled.

  double* xp = (double*) x.Values();
  double* yp = (double*) y.Values();
  const Epetra_Import* Importer_=A.Importer();
  const Epetra_Export* Exporter_=A.Exporter();
  Epetra_Vector *xcopy=0, *ImportVector_=0, *ExportVector_=0;

  if (&x==&y && Importer_==0 && Exporter_==0) {
    xcopy = new Epetra_Vector(x);
    xp = (double *) xcopy->Values();
  }
  else if (Importer_)
    ImportVector_ = new Epetra_Vector(Importer_->TargetMap());
  else if (Exporter_)
    ExportVector_ = new Epetra_Vector(Exporter_->SourceMap());
  

  // If we have a non-trivial importer, we must import elements that are permuted or are on other processors
  if(Importer_ != 0) {
    EPETRA_CHK_ERR(ImportVector_->Import(x, *Importer_, Insert));
    xp = (double*) ImportVector_->Values();
    }
  
  // If we have a non-trivial exporter, we must export elements that are permuted or belong to other processors
  if(Exporter_ != 0)  yp = (double*) ExportVector_->Values();
  
  // Do actual computation
  for(int i = 0; i < A.NumMyRows(); i++) {
    int NumEntries,*RowIndices;
    A.Graph().ExtractMyRowView(i,NumEntries,RowIndices);
    double sum = 0.0;
    for(int j = 0; j < NumEntries; j++) 
      sum += xp[*RowIndices++];    
    yp[i] = sum;    
  }
  
  if(Exporter_ != 0) {
    y.PutScalar(0.0); // Make sure target is zero
    EPETRA_CHK_ERR(y.Export(*ExportVector_, *Exporter_, Add)); // Fill y with Values from export vector
  }
  // Handle case of rangemap being a local replicated map
  if (!A.Graph().RangeMap().DistributedGlobal() && A.Comm().NumProc()>1) EPETRA_CHK_ERR(y.Reduce());

  delete xcopy;
  delete ImportVector_;
  delete ExportVector_;


  return(0);
}

/*************************************************************************************/
/*************************************************************************************/
/*************************************************************************************/
void solution_test(string msg, const Epetra_Operator &A,const Epetra_MultiVector &lhs,const Epetra_MultiVector &rhs,const Epetra_MultiVector &xexact,Epetra_Time & Time, double & TotalErrorExactSol, double& TotalErrorResidual){
  // ==================================================== //  
  // compute difference between exact solution and ML one //
  // ==================================================== //  
  double d = 0.0, d_tot = 0.0;  
  for( int i=0 ; i<lhs.Map().NumMyElements() ; ++i )
    d += (lhs[0][i] - xexact[0][i]) * (lhs[0][i] - xexact[0][i]);
  
  A.Comm().SumAll(&d,&d_tot,1);
  
  // ================== //
  // compute ||Ax - b|| //
  // ================== //
  double Norm;
  Epetra_Vector Ax(rhs.Map());
  A.Apply(lhs, Ax);
  Ax.Update(1.0, rhs, -1.0);
  Ax.Norm2(&Norm);
  
  if (A.Comm().MyPID() == 0) {
    cout << msg << "......Using " << A.Comm().NumProc() << " processes" << endl;
    cout << msg << "......||A x - b||_2 = " << Norm << endl;
    cout << msg << "......||x_exact - x||_2 = " << sqrt(d_tot) << endl;
    cout << msg << "......Total Time = " << Time.ElapsedTime() << endl;
  }
  
  TotalErrorExactSol += sqrt(d_tot);
  TotalErrorResidual += Norm;
}


void TestMultiLevelPreconditioner_CurlLSFEM(char ProblemType[],
                                           Teuchos::ParameterList   & MLList,
                                           Epetra_CrsMatrix   & CurlCurl,
                                           Epetra_CrsMatrix   & D0clean,
                                           Epetra_CrsMatrix   & M0inv,
                                           Epetra_CrsMatrix   & M1,
                                           Epetra_MultiVector & xh,
                                           Epetra_MultiVector & b,
                                           double & TotalErrorResidual,
                                           double & TotalErrorExactSol){
  
  /* Nuke M1 for D0, OAZ*/
  Epetra_CrsMatrix D0(D0clean);                                             
  ML_Epetra::Apply_BCsToGradient(CurlCurl,D0);  
  
  /* Get the BC edges*/
  int numBCedges;  
  int* BCedges=ML_Epetra::FindLocalDiricheltRowsFromOnesAndZeros(CurlCurl,numBCedges);  
  //  printf("# BC edges = %d\n",numBCedges);
  ML_Epetra::Apply_OAZToMatrix(BCedges,numBCedges,M1);

  if(!CurlCurl.Comm().MyPID())
    cout<<"Total number of rows = "<<CurlCurl.NumGlobalRows()<<endl;

  /* Build the (1,1) Block Operator */
  ML_Epetra::ML_RefMaxwell_11_Operator Operator11(CurlCurl,D0,M0inv,M1);
  
  /* Build the AztecOO stuff */
  Epetra_MultiVector x(xh);
  x.PutScalar(0.0);
  
  Epetra_LinearProblem Problem(&Operator11,&x,&b); 
  Epetra_MultiVector* lhs = Problem.GetLHS();
  Epetra_MultiVector* rhs = Problem.GetRHS();
  
  Epetra_Time Time(CurlCurl.Comm());
    
  /* Build the aggregation guide matrix */
  Epetra_CrsMatrix *TMT_Agg_Matrix;
  ML_Epetra::ML_Epetra_PtAP(M1,D0clean,TMT_Agg_Matrix,false);
  

  /* Approximate the diagonal for EMFP: 2a^2 b guy */
  Epetra_Vector Diagonal(CurlCurl.DomainMap());
  Epetra_Vector NodeDiagonal(D0.DomainMap());
  Epetra_Vector EdgeDiagonal(M1.DomainMap());
  Epetra_Vector CurlDiagonal(CurlCurl.DomainMap());

  M0inv.ExtractDiagonalCopy(NodeDiagonal);
  M1.ExtractDiagonalCopy(EdgeDiagonal);
  CurlCurl.ExtractDiagonalCopy(CurlDiagonal);

  Multiply_Ones(D0,NodeDiagonal,Diagonal);

  for(int i=0;i<CurlCurl.NumMyRows();i++) {
    Diagonal[i]=Diagonal[i]*(EdgeDiagonal[i]*EdgeDiagonal[i]) + CurlDiagonal[i];
    if(ABS(Diagonal[i])<1e-12) Diagonal[i]=1.0;
  }

  /* Build the EMFP Preconditioner */  
  ML_Epetra::EdgeMatrixFreePreconditioner EMFP(Operator11,Diagonal,D0,D0clean,*TMT_Agg_Matrix,BCedges,numBCedges,MLList);

  /* Solve! */
  AztecOO solver(Problem);  
  solver.SetPrecOperator(&EMFP);
  solver.SetAztecOption(AZ_solver, AZ_cg);
  solver.SetAztecOption(AZ_output, 32);
  solver.Iterate(500, 1e-10);
  //  solver.Iterate(1, 1e-10);

  Epetra_MultiVector xexact(xh);
  xexact.PutScalar(0.0);
  
  // accuracy check
  string msg = ProblemType;
  solution_test(msg,Operator11,*lhs,*rhs,xexact,Time,TotalErrorExactSol,TotalErrorResidual);


#ifdef DUMP_DATA
   EpetraExt::RowMatrixToMatlabFile("mag_m0inv_matrix.dat",M0inv);
   EpetraExt::RowMatrixToMatlabFile("mag_m1_matrix.dat",M1);
   EpetraExt::RowMatrixToMatlabFile("mag_k1_matrix.dat",CurlCurl);
   EpetraExt::RowMatrixToMatlabFile("mag_t0_matrix.dat",D0);
   EpetraExt::RowMatrixToMatlabFile("mag_t0_clean_matrix.dat",D0clean);
   EpetraExt::MultiVectorToMatrixMarketFile("mag_rhs1.dat",*rhs,0,0,false);
   EpetraExt::MultiVectorToMatrixMarketFile("mag_lhs1.dat",*lhs,0,0,false);
#endif

    xh = *lhs;

    // Cleanup
    delete TMT_Agg_Matrix;
    delete [] BCedges;
}

// Calculates value of exact solution u
 int evalu(double & uExact0, double & uExact1, double & uExact2, double & x, double & y, double & z)
 {
    uExact0 = exp(x+y+z)*(y+1.0)*(y-1.0)*(z+1.0)*(z-1.0);
    uExact1 = exp(x+y+z)*(x+1.0)*(x-1.0)*(z+1.0)*(z-1.0);
    uExact2 = exp(x+y+z)*(x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);
 /*
    
    uExact0 = cos(M_PI*x)*exp(y*z)*(y+1.0)*(y-1.0)*(z+1.0)*(z-1.0);
    uExact1 = cos(M_PI*y)*exp(x*z)*(x+1.0)*(x-1.0)*(z+1.0)*(z-1.0);
    uExact2 = cos(M_PI*z)*exp(x*y)*(x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);
   
    uExact0 = cos(M_PI*x)*sin(M_PI*y)*sin(M_PI*z);
    uExact1 = sin(M_PI*x)*cos(M_PI*y)*sin(M_PI*z);
    uExact2 = sin(M_PI*x)*sin(M_PI*y)*cos(M_PI*z);
 
*/
 /*
 
    uExact0 = x*(y*y - 1.0)*(z*z-1.0);
    uExact1 = y*(x*x - 1.0)*(z*z-1.0);
    uExact2 = z*(x*x - 1.0)*(y*y-1.0);

    uExact0 = (y*y - 1.0)*(z*z-1.0);
    uExact1 = (x*x - 1.0)*(z*z-1.0);
    uExact2 = (x*x - 1.0)*(y*y-1.0);
 
 */
  
   return 0;
 }

// Calculates divergence of exact solution u
 double evalDivu(double & x, double & y, double & z, double & mu)
 {
  
   double divu = exp(x+y+z)*(y+1.0)*(y-1.0)*(z+1.0)*(z-1.0)
                 + exp(x+y+z)*(x+1.0)*(x-1.0)*(z+1.0)*(z-1.0)
                 + exp(x+y+z)*(x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);
 /*

   double divu = -M_PI*sin(M_PI*x)*exp(y*z)*(y+1.0)*(y-1.0)*(z+1.0)*(z-1.0)
                 -M_PI*sin(M_PI*y)*exp(x*z)*(x+1.0)*(x-1.0)*(z+1.0)*(z-1.0)
                 -M_PI*sin(M_PI*z)*exp(x*y)*(x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);

   double divu = -3.0*M_PI*sin(M_PI*x)*sin(M_PI*y)*sin(M_PI*z);
  */

/*
   double divu = (y+1.0)*(y-1.0)*(z+1.0)*(z-1.0)
                 + (x+1.0)*(x-1.0)*(z+1.0)*(z-1.0)
                 + (x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);
*/

  // double divu = 0.0;

   return mu*divu;
 }


// Calculates curl of exact solution u
 int evalCurlu(double & curlu0, double & curlu1, double & curlu2, double & x, double & y, double & z, double &mu)
 {
  
   double duxdy = exp(x+y+z)*(z*z-1.0)*(y*y+2.0*y-1.0);
   double duxdz = exp(x+y+z)*(y*y-1.0)*(z*z+2.0*z-1.0);
   double duydx = exp(x+y+z)*(z*z-1.0)*(x*x+2.0*x-1.0);
   double duydz = exp(x+y+z)*(x*x-1.0)*(z*z+2.0*z-1.0);
   double duzdx = exp(x+y+z)*(y*y-1.0)*(x*x+2.0*x-1.0);
   double duzdy = exp(x+y+z)*(x*x-1.0)*(y*y+2.0*y-1.0);

 /*
 
   double duxdy = cos(M_PI*x)*exp(y*z)*(z+1.0)*(z-1.0)*(z*(y+1.0)*(y-1.0) + 2.0*y);
   double duxdz = cos(M_PI*x)*exp(y*z)*(y+1.0)*(y-1.0)*(y*(z+1.0)*(z-1.0) + 2.0*z);
   double duydx = cos(M_PI*y)*exp(x*z)*(z+1.0)*(z-1.0)*(z*(x+1.0)*(x-1.0) + 2.0*x);
   double duydz = cos(M_PI*y)*exp(x*z)*(x+1.0)*(x-1.0)*(x*(z+1.0)*(z-1.0) + 2.0*z);
   double duzdx = cos(M_PI*z)*exp(x*y)*(y+1.0)*(y-1.0)*(y*(x+1.0)*(x-1.0) + 2.0*x);
   double duzdy = cos(M_PI*z)*exp(x*y)*(x+1.0)*(x-1.0)*(x*(y+1.0)*(y-1.0) + 2.0*y);

 
   double duxdy = M_PI*cos(M_PI*x)*cos(M_PI*y)*sin(M_PI*z);
   double duxdz = M_PI*cos(M_PI*x)*sin(M_PI*y)*cos(M_PI*z);
   double duydx = M_PI*cos(M_PI*x)*cos(M_PI*y)*sin(M_PI*z);
   double duydz = M_PI*sin(M_PI*x)*cos(M_PI*y)*cos(M_PI*z);
   double duzdx = M_PI*cos(M_PI*x)*sin(M_PI*y)*cos(M_PI*z);
   double duzdy = M_PI*sin(M_PI*x)*cos(M_PI*y)*cos(M_PI*z);
 
 */

 /*
   double duxdy = 2.0*x*y*(z*z-1);
   double duxdz = 2.0*x*z*(y*y-1);
   double duydx = 2.0*y*x*(z*z-1);
   double duydz = 2.0*y*z*(x*x-1);
   double duzdx = 2.0*z*x*(y*y-1);
   double duzdy = 2.0*z*y*(x*x-1);
  
   double duxdy = 2.0*y*(z*z-1);
   double duxdz = 2.0*z*(y*y-1);
   double duydx = 2.0*x*(z*z-1);
   double duydz = 2.0*z*(x*x-1);
   double duzdx = 2.0*x*(y*y-1);
   double duzdy = 2.0*y*(x*x-1);
   
*/

   curlu0 = mu*(duzdy - duydz);
   curlu1 = mu*(duxdz - duzdx);
   curlu2 = mu*(duydx - duxdy);

   return 0;
 }

// Calculates gradient of the divergence of exact solution u
 int evalGradDivu(double & gradDivu0, double & gradDivu1, double & gradDivu2, double & x, double & y, double & z, double & mu)
{
   
 
   gradDivu0 = exp(x+y+z)*((y*y-1.0)*(z*z-1.0)+(x*x+2.0*x-1.0)*(z*z-1.0)+(x*x+2.0*x-1.0)*(y*y-1.0));
   gradDivu1 = exp(x+y+z)*((y*y+2.0*y-1.0)*(z*z-1.0)+(x*x-1.0)*(z*z-1.0)+(x*x-1.0)*(y*y+2.0*y-1.0));
   gradDivu2 = exp(x+y+z)*((y*y-1.0)*(z*z+2.0*z-1.0)+(x*x-1.0)*(z*z+2.0*z-1.0)+(x*x-1.0)*(y*y-1.0));
 
  /*
   
    gradDivu0 = -M_PI*M_PI*cos(M_PI*x)*exp(y*z)*(y+1.0)*(y-1.0)*(z+1.0)*(z-1.0)
                  -M_PI*sin(M_PI*y)*exp(x*z)*(z+1.0)*(z-1.0)*(z*(x+1.0)*(x-1.0)+2.0*x)
                  -M_PI*sin(M_PI*z)*exp(x*y)*(y+1.0)*(y-1.0)*(y*(x+1.0)*(x-1.0)+2.0*x);
    gradDivu1 = -M_PI*sin(M_PI*x)*exp(y*z)*(z+1.0)*(z-1.0)*(z*(y+1.0)*(y-1.0)+2.0*y)
                  -M_PI*M_PI*cos(M_PI*y)*exp(x*z)*(x+1.0)*(x-1.0)*(z+1.0)*(z-1.0)
                  -M_PI*sin(M_PI*z)*exp(x*y)*(x+1.0)*(x-1.0)*(x*(y+1.0)*(y-1.0)+2.0*y);
    gradDivu2 = -M_PI*sin(M_PI*x)*exp(y*z)*(y+1.0)*(y-1.0)*(y*(z+1.0)*(z-1.0)+2.0*z)
                  -M_PI*sin(M_PI*y)*exp(x*z)*(x+1.0)*(x-1.0)*(x*(z+1.0)*(z-1.0)+2.0*z)
                  -M_PI*M_PI*cos(M_PI*z)*exp(x*y)*(x+1.0)*(x-1.0)*(y+1.0)*(y-1.0);
  

   gradDivu0 = -3.0*M_PI*M_PI*cos(M_PI*x)*sin(M_PI*y)*sin(M_PI*z);
   gradDivu1 = -3.0*M_PI*M_PI*sin(M_PI*x)*cos(M_PI*y)*sin(M_PI*z);
   gradDivu2 = -3.0*M_PI*M_PI*sin(M_PI*x)*sin(M_PI*y)*cos(M_PI*z);

 */

 /*
   gradDivu0 = 2.0*x*((z*z-1.0)+(y*y-1.0));
   gradDivu1 = 2.0*y*((z*z-1.0)+(x*x-1.0));
   gradDivu2 = 2.0*z*((x*x-1.0)+(y*y-1.0));

 
   gradDivu0 = 0;
   gradDivu1 = 0;
   gradDivu2 = 0;
  */

   gradDivu0 = gradDivu0 * mu;
   gradDivu1 = gradDivu1 * mu;
   gradDivu2 = gradDivu2 * mu;
   

   return 0;
}



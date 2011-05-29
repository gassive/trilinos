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
// Questions? Contact Pavel Bochev (pbboche@sandia.gov) or
//                    Denis Ridzal (dridzal@sandia.gov)
//                    Kara Peterson (kjpeter@sandia.gov).
//
// ************************************************************************
// @HEADER

/** \file   Intrepid_HDIV_TRI_I1_FEMDef.hpp
    \brief  Definition file for FEM basis functions of degree 1 for H(div) functions on Triangle cells.
    \author Created by P. Bochev, D. Ridzal and K. Peterson.
*/

namespace Intrepid {

template<class Scalar, class ArrayScalar>
Basis_HDIV_TRI_I1_FEM<Scalar,ArrayScalar>::Basis_HDIV_TRI_I1_FEM()
  {
    this -> basisCardinality_  = 3;
    this -> basisDegree_       = 1;
    this -> basisCellTopology_ = shards::CellTopology(shards::getCellTopologyData<shards::Triangle<3> >() );
    this -> basisType_         = BASIS_FEM_DEFAULT;
    this -> basisCoordinates_  = COORDINATES_CARTESIAN;
    this -> basisTagsAreSet_   = false;
  }
  
template<class Scalar, class ArrayScalar>
void Basis_HDIV_TRI_I1_FEM<Scalar, ArrayScalar>::initializeTags() {
  
  // Basis-dependent intializations
  int tagSize  = 4;        // size of DoF tag
  int posScDim = 0;        // position in the tag, counting from 0, of the subcell dim 
  int posScOrd = 1;        // position in the tag, counting from 0, of the subcell ordinal
  int posDfOrd = 2;        // position in the tag, counting from 0, of DoF ordinal relative to the subcell

  // An array with local DoF tags assigned to basis functions, in the order of their local enumeration 
  int tags[]  = {
                  1, 0, 0, 1,
                  1, 1, 0, 1,
                  1, 2, 0, 1
                  // 2, 0, 0, 1,
                  // 2, 1, 0, 1,
                  // 2, 2, 0, 1
                };
  
  // Basis-independent function sets tag and enum data in tagToOrdinal_ and ordinalToTag_ arrays:
  Intrepid::setOrdinalTagData(this -> tagToOrdinal_,
                              this -> ordinalToTag_,
                              tags,
                              this -> basisCardinality_,
                              tagSize,
                              posScDim,
                              posScOrd,
                              posDfOrd);
}



template<class Scalar, class ArrayScalar>
void Basis_HDIV_TRI_I1_FEM<Scalar, ArrayScalar>::getValues(ArrayScalar &        outputValues,
                                                           const ArrayScalar &  inputPoints,
                                                           const EOperator      operatorType) const {
  

// Verify arguments
#ifdef HAVE_INTREPID_DEBUG
  Intrepid::getValues_HDIV_Args<Scalar, ArrayScalar>(outputValues,
                                                     inputPoints,
                                                     operatorType,
                                                     this -> getBaseCellTopology(),
                                                     this -> getCardinality() );
#endif

  
 // Number of evaluation points = dim 0 of inputPoints
  int dim0 = inputPoints.dimension(0);

  // Temporaries: (x,y) coordinates of the evaluation point
  Scalar x = 0.0;                                    
  Scalar y = 0.0;                                    
  
  switch (operatorType) {
    case OPERATOR_VALUE:
      for (int i0 = 0; i0 < dim0; i0++) {
        x = inputPoints(i0, 0);
        y = inputPoints(i0, 1);
        
        // outputValues is a rank-3 array with dimensions (basisCardinality_, dim0, spaceDim)
        outputValues(0, i0, 0) = x;
        outputValues(0, i0, 1) = y - 1.0;

        outputValues(1, i0, 0) = x;
        outputValues(1, i0, 1) = y;

        outputValues(2, i0, 0) = x - 1.0;
        outputValues(2, i0, 1) = y;
      }
      break;

    case OPERATOR_DIV:
      // outputValues is a rank-2 array with dimensions (basisCardinality_, dim0)
      for (int i0 = 0; i0 < dim0; i0++) {
         outputValues(0, i0) = 2.0;
         outputValues(1, i0) = 2.0;
         outputValues(2, i0) = 2.0;
       }
      break;

    case OPERATOR_CURL:
       TEST_FOR_EXCEPTION( (operatorType == OPERATOR_CURL), std::invalid_argument,
                          ">>> ERROR (Basis_HDIV_TRI_I1_FEM): CURL is invalid operator for HDIV Basis Functions");
      break;
      
    case OPERATOR_GRAD:
       TEST_FOR_EXCEPTION( (operatorType == OPERATOR_GRAD), std::invalid_argument,
                          ">>> ERROR (Basis_HDIV_TRI_I1_FEM): GRAD is invalid operator for HDIV Basis Functions");
      break;

    case OPERATOR_D1:
    case OPERATOR_D2:
    case OPERATOR_D3:
    case OPERATOR_D4:
    case OPERATOR_D5:
    case OPERATOR_D6:
    case OPERATOR_D7:
    case OPERATOR_D8:
    case OPERATOR_D9:
    case OPERATOR_D10:
      TEST_FOR_EXCEPTION( ( (operatorType == OPERATOR_D1)    &&
                            (operatorType == OPERATOR_D2)    &&
                            (operatorType == OPERATOR_D3)    &&
                            (operatorType == OPERATOR_D4)    &&
                            (operatorType == OPERATOR_D5)    &&
                            (operatorType == OPERATOR_D6)    &&
                            (operatorType == OPERATOR_D7)    &&
                            (operatorType == OPERATOR_D8)    &&
                            (operatorType == OPERATOR_D9)    &&
                            (operatorType == OPERATOR_D10) ),
                          std::invalid_argument,
                          ">>> ERROR (Basis_HDIV_TRI_I1_FEM): Invalid operator type");
      break;
      
    default:
      TEST_FOR_EXCEPTION( ( (operatorType != OPERATOR_VALUE) &&
                            (operatorType != OPERATOR_GRAD)  &&
                            (operatorType != OPERATOR_CURL)  &&
                            (operatorType != OPERATOR_DIV)   &&
                            (operatorType != OPERATOR_D1)    &&
                            (operatorType != OPERATOR_D2)    &&
                            (operatorType != OPERATOR_D3)    &&
                            (operatorType != OPERATOR_D4)    &&
                            (operatorType != OPERATOR_D5)    &&
                            (operatorType != OPERATOR_D6)    &&
                            (operatorType != OPERATOR_D7)    &&
                            (operatorType != OPERATOR_D8)    &&
                            (operatorType != OPERATOR_D9)    &&
                            (operatorType != OPERATOR_D10) ),
                          std::invalid_argument,
                          ">>> ERROR (Basis_HDIV_TRI_I1_FEM): Invalid operator type");
  }
}


  
template<class Scalar, class ArrayScalar>
void Basis_HDIV_TRI_I1_FEM<Scalar, ArrayScalar>::getValues(ArrayScalar&           outputValues,
                                                            const ArrayScalar &    inputPoints,
                                                            const ArrayScalar &    cellVertices,
                                                            const EOperator        operatorType) const {
  TEST_FOR_EXCEPTION( (true), std::logic_error,
                      ">>> ERROR (Basis_HDIV_TRI_I1_FEM): FEM Basis calling an FVD member function");
                                                             }

}// namespace Intrepid

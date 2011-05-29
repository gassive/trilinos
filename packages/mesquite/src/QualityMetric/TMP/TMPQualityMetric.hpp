/* ***************************************************************** 
    MESQUITE -- The Mesh Quality Improvement Toolkit

    Copyright 2006 Sandia National Laboratories.  Developed at the
    University of Wisconsin--Madison under SNL contract number
    624796.  The U.S. Government and the University of Wisconsin
    retain certain rights to this software.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License 
    (lgpl.txt) along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 
    (2006) kraftche@cae.wisc.edu
   
  ***************************************************************** */


/** \file TMPQualityMetric.hpp
 *  \brief 
 *  \author Jason Kraftcheck 
 */

#ifndef MSQ_JACOBIAN_METRIC_HPP
#define MSQ_JACOBIAN_METRIC_HPP

#include "Mesquite.hpp"
#include "ElemSampleQM.hpp"
#include "MsqMatrix.hpp"

namespace MESQUITE_NS {

class TargetCalculator;
class WeightCalculator;
class TargetMetric2D;
class TargetMetric3D;
class NodeSet;
class Mesh;
class MeshDomain;
class Settings;

/**\brief Compare targets to mapping function Jacobian matrices
 *
 * A quality metric defined using 2D and 3D target metrics,
 * where the active (A) matrix compared to the target by
 * the underlying metrics is the Jacobian matrix of the
 * mapping function at a given sample point.  For surface
 * elements, A is rotated to align the normal with W, such that
 * both matrices can be reduced from 3x2 to 2x2.
 */
class TMPQualityMetric : public ElemSampleQM
{
public:

  /**
   *\param tc   The target calculator 
   *\param wc   The weight calculator
   *\param metric_2d Metric to use for surface elements - may be NULL
   *            if mesh contains only volume elements.
   *\param metric_3d Metric to use for volume elements - may be NULL
   *            if mesh contains only surface elements.
   */
  TMPQualityMetric( TargetCalculator* tc,
                    WeightCalculator* wc,
                    TargetMetric2D* metric_2d,
                    TargetMetric3D* metric_3d ) 
    : targetCalc(tc),
      weightCalc(wc),
      metric2D( metric_2d ),
      metric3D( metric_3d )
   {}

  /**
   *\param tc   The target calculator 
   *\param metric_2d Metric to use for surface elements - may be NULL
   *            if mesh contains only volume elements.
   *\param metric_3d Metric to use for volume elements - may be NULL
   *            if mesh contains only surface elements.
   */
  TMPQualityMetric( TargetCalculator* tc,
                    TargetMetric2D* metric_2d,
                    TargetMetric3D* metric_3d ) 
    : targetCalc(tc),
      weightCalc(0),
      metric2D( metric_2d ),
      metric3D( metric_3d )
   {}
     
  MESQUITE_EXPORT virtual
  std::string get_name() const;
  
  MESQUITE_EXPORT virtual 
  int get_negate_flag() const;
  
  MESQUITE_EXPORT virtual
  void get_evaluations( PatchData& pd, 
                        std::vector<size_t>& handles, 
                        bool free_vertices_only,
                        MsqError& err );
  
  MESQUITE_EXPORT static
  void get_patch_evaluations( PatchData& pd, 
                        std::vector<size_t>& handles, 
                        bool free_vertices_only,
                        MsqError& err );
  
  MESQUITE_EXPORT virtual 
  void get_element_evaluations( PatchData& pd, size_t elem_index,
                                std::vector<size_t>& handles,
                                MsqError& err );
  
  MESQUITE_EXPORT virtual
  bool evaluate( PatchData& pd, 
                 size_t handle, 
                 double& value, 
                 MsqError& err );

  MESQUITE_EXPORT virtual
  bool evaluate_with_indices( PatchData& pd,
                 size_t handle,
                 double& value,
                 std::vector<size_t>& indices,
                 MsqError& err );
                 
  MESQUITE_EXPORT virtual
  bool evaluate_with_gradient( PatchData& pd,
                 size_t handle,
                 double& value,
                 std::vector<size_t>& indices,
                 std::vector<Vector3D>& gradient,
                 MsqError& err );

  MESQUITE_EXPORT virtual
  bool evaluate_with_Hessian_diagonal( PatchData& pd,
                    size_t handle,
                    double& value,
                    std::vector<size_t>& indices,
                    std::vector<Vector3D>& gradient,
                    std::vector<SymMatrix3D>& Hessian_diagonal,
                    MsqError& err );
                    
  MESQUITE_EXPORT virtual
  bool evaluate_with_Hessian( PatchData& pd,
                    size_t handle,
                    double& value,
                    std::vector<size_t>& indices,
                    std::vector<Vector3D>& gradient,
                    std::vector<Matrix3D>& Hessian,
                    MsqError& err );
    
  void set_target_calculator( TargetCalculator* tc ) { targetCalc = tc; }
  void set_weight_calculator( WeightCalculator* wc ) { weightCalc = wc; }
  TargetCalculator* get_target_calculator() const { return targetCalc; }
  WeightCalculator* get_weight_calculator() const { return weightCalc; }
  
  TargetMetric2D* get_2d_metric() const { return metric2D; }
  TargetMetric3D* get_3d_metric() const { return metric3D; }
  void set_2d_metric( TargetMetric2D* m ) { metric2D = m; }
  void set_3d_metric( TargetMetric3D* m ) { metric3D = m; }
    
  virtual void initialize_queue( Mesh* mesh,
                                 MeshDomain* domain,
                                 Settings* settings,
                                 MsqError& err );
  
private:

  bool evaluate_with_indices( PatchData& pd,
                 size_t handle,
                 double& value,
                 size_t* indices,
                 size_t& num_indices,
                 MsqError& err );

  bool evaluate_surface_common( // input:
                                PatchData& pd,
                                Sample sample,
                                size_t element_index,
                                const NodeSet& bits,
                                // output:
                                size_t* indices, 
                                size_t& num_indices,
                                MsqVector<2>* derivs,
                                MsqMatrix<2,2>& W,
                                MsqMatrix<2,2>& A,
                                MsqMatrix<3,2>& S_a_transpose_Theta,
                                MsqError& err );
                                

  TargetCalculator* targetCalc;
  WeightCalculator* weightCalc;
  TargetMetric2D* metric2D;
  TargetMetric3D* metric3D;
  
  enum { MAX_ELEM_NODES = 27 };
  size_t mIndices[MAX_ELEM_NODES];
  std::vector< MsqMatrix<2,2> > hess2d;
  MsqVector<3> mDerivs3D[MAX_ELEM_NODES];
  MsqVector<2> mDerivs2D[MAX_ELEM_NODES];
};

} // namespace Mesquite

#endif

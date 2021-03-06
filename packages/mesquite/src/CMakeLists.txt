IF (NOT ${PROJECT_NAME} STREQUAL "Mesquite")

  INCLUDE(PackageLibraryMacros)

  #
  # A) Package-specific configuration options
  #

ELSE()
  MACRO(APPEND_SET VARNAME)
    SET(${VARNAME} ${${VARNAME}} ${ARGN})
  ENDMACRO()
  
  FUNCTION(PACKAGE_ADD_LIBRARY name)
    LIST(REMOVE_ITEM ARGV "${name}" "HEADERS" "SOURCES")
    IF(${PACKAGE_NAME}_ENABLE_SHARED)
      ADD_LIBRARY( ${name} SHARED ${ARGV} )
    ELSE()
      ADD_LIBRARY( ${name} STATIC ${ARGV} )
    ENDIF()
  ENDFUNCTION()
ENDIF()

#
# B) Define the header and source files (and directories)
#

#
# src
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../include)

APPEND_SET(HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/../include/mesquite_config.h
  ${CMAKE_CURRENT_BINARY_DIR}/../include/mesquite_version.h
  ../include/Mesquite.hpp
  ../include/MeshInterface.hpp
  ../include/ParallelMeshInterface.hpp
  ../include/ParallelHelperInterface.hpp
  )

#
# src/Mesh
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Mesh)

IF (TPL_ENABLE_MPI)
  APPEND_SET(HEADERS
    Mesh/ParallelHelper.hpp
    )

  APPEND_SET(SOURCES
    Mesh/ParallelHelper.cpp
    )
ENDIF()

APPEND_SET(HEADERS
  Mesh/ArrayMesh.hpp
  Mesh/ElementPatches.hpp
  Mesh/ExtraData.hpp
  Mesh/ExtraDataUser.hpp
  Mesh/GlobalPatch.hpp
  Mesh/IdealElements.hpp
  Mesh/MeshDecorator.hpp
  Mesh/MeshImpl.hpp
  Mesh/MeshImplData.hpp
  Mesh/MeshImplTags.hpp
  Mesh/MeshWriter.hpp
  Mesh/MsqFreeVertexIndexIterator.hpp
  Mesh/MsqMeshEntity.hpp
  Mesh/MsqVertex.hpp
  Mesh/ParallelMeshImpl.hpp
  Mesh/PatchData.hpp
  Mesh/PatchIterator.hpp
  Mesh/PatchSet.hpp
  Mesh/TagVertexMesh.hpp
  Mesh/TopologyInfo.hpp
  Mesh/VertexPatches.hpp
  )

APPEND_SET(SOURCES
  Mesh/ArrayMesh.cpp
  Mesh/ElementPatches.cpp
  Mesh/ExtraData.cpp
  Mesh/GlobalPatch.cpp
  Mesh/IdealElements.cpp
  Mesh/MeshDecorator.cpp
  Mesh/MeshImpl.cpp
  Mesh/MeshImplData.cpp
  Mesh/MeshImplTags.cpp
  Mesh/MeshWriter.cpp
  Mesh/MsqMeshEntity.cpp
  Mesh/MsqVertex.cpp
  Mesh/ParallelMeshImpl.cpp
  Mesh/PatchData.cpp
  Mesh/PatchIterator.cpp
  Mesh/TagVertexMesh.cpp
  Mesh/TopologyInfo.cpp
  Mesh/VertexPatches.cpp
  )

#
# src/Control
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Control)

APPEND_SET(HEADERS
  Control/Instruction.hpp
  Control/InstructionQueue.hpp
  Control/IQInterface.hpp
  Control/TerminationCriterion.hpp
  Control/Settings.hpp
  Control/SlaveBoundaryVertices.hpp
  Control/VertexSlaver.hpp 
  )

APPEND_SET(SOURCES
  Control/Instruction.cpp
  Control/InstructionQueue.cpp
  Control/TerminationCriterion.cpp
  Control/Settings.cpp
  Control/SlaveBoundaryVertices.cpp 
  )


#
# src/Control/Wrappers
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Control/Wrappers)

APPEND_SET(HEADERS
  Control/Wrappers/LaplacianIQ.hpp
  Control/Wrappers/ShapeImprovementWrapper.hpp
  Control/Wrappers/SizeAdaptShapeWrapper.hpp
  Control/Wrappers/PaverMinEdgeLengthWrapper.hpp 
  Control/Wrappers/ViscousCFDTetShapeWrapper.hpp 
  Control/Wrappers/Wrapper.hpp 
  )

APPEND_SET(SOURCES
  Control/Wrappers/LaplacianIQ.cpp
  Control/Wrappers/ShapeImprovementWrapper.cpp
  Control/Wrappers/SizeAdaptShapeWrapper.cpp
  Control/Wrappers/PaverMinEdgeLengthWrapper.cpp
  Control/Wrappers/ViscousCFDTetShapeWrapper.cpp
  Control/Wrappers/Wrapper.cpp 
  )

#
# src/MappingFunction
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/MappingFunction)

APPEND_SET(HEADERS
  MappingFunction/MappingFunction.hpp
  MappingFunction/NodeSet.hpp
  MappingFunction/Sample.hpp
  )

APPEND_SET(SOURCES
  MappingFunction/MappingFunction.cpp
  MappingFunction/NodeSet.cpp
  MappingFunction/Sample.cpp
  )

#
# src/Control/MappingFunction/Lagrange
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/MappingFunction/Lagrange)

APPEND_SET(HEADERS
  MappingFunction/Lagrange/HexLagrangeShape.hpp
  MappingFunction/Lagrange/QuadLagrangeShape.hpp
  MappingFunction/Lagrange/TetLagrangeShape.hpp
  MappingFunction/Lagrange/TriLagrangeShape.hpp 
  )

APPEND_SET(SOURCES
  MappingFunction/Lagrange/HexLagrangeShape.cpp
  MappingFunction/Lagrange/QuadLagrangeShape.cpp
  MappingFunction/Lagrange/TetLagrangeShape.cpp
  MappingFunction/Lagrange/TriLagrangeShape.cpp 
  )

#
# src/Control/MappingFunction/Linear
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/MappingFunction/Linear)

APPEND_SET(HEADERS
  MappingFunction/Linear/LinearHexahedron.hpp
  MappingFunction/Linear/LinearPrism.hpp
  MappingFunction/Linear/LinearPyramid.hpp
  MappingFunction/Linear/LinearQuadrilateral.hpp
  MappingFunction/Linear/LinearTetrahedron.hpp
  MappingFunction/Linear/LinearTriangle.hpp 
  )

APPEND_SET(SOURCES
  MappingFunction/Linear/LinearHexahedron.cpp
  MappingFunction/Linear/LinearPrism.cpp
  MappingFunction/Linear/LinearPyramid.cpp
  MappingFunction/Linear/LinearQuadrilateral.cpp
  MappingFunction/Linear/LinearTetrahedron.cpp
  MappingFunction/Linear/LinearTriangle.cpp 
  )

#
# src/Misc
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/Misc)

APPEND_SET(HEADERS
  Misc/BoundedCylinderDomain.hpp
  Misc/Bits.hpp
  Misc/ConicDomain.hpp
  Misc/CylinderDomain.hpp
  Misc/DomainClassifier.hpp
  Misc/Exponent.hpp
  Misc/FileTokenizer.hpp
  Misc/Matrix3D.hpp
  Misc/MeshDomain1D.hpp
  Misc/MeshTransform.hpp
  Misc/MsqDebug.hpp
  Misc/MsqError.hpp
  Misc/MsqFPE.hpp
  Misc/MsqGeomPrim.hpp
  Misc/MsqHessian.hpp
  Misc/MsqInterrupt.hpp
  Misc/MsqMatrix.hpp
  Misc/MsqTimer.hpp
  Misc/PlanarDomain.hpp
  Misc/SphericalDomain.hpp
  Misc/SymMatrix3D.hpp
  Misc/Vector3D.hpp
  Misc/VtkTypeInfo.hpp
  Misc/XYRectangle.hpp
  )

APPEND_SET(SOURCES
  Misc/BoundedCylinderDomain.cpp
  Misc/ConicDomain.cpp
  Misc/CylinderDomain.cpp
  Misc/DomainClassifier.cpp
  Misc/DomainUtil.cpp
  Misc/DomainUtil.hpp
  Misc/Exponent.cpp
  Misc/FileTokenizer.cpp
  Misc/MeshDomain1D.cpp
  Misc/MeshTransform.cpp
  Misc/MesquiteVersion.cpp
  Misc/MsqDebug.cpp
  Misc/MsqError.cpp
  Misc/MsqFPE.cpp
  Misc/MsqGeomPrim.cpp
  Misc/MsqHessian.cpp
  Misc/MsqInterrupt.cpp
  Misc/MsqTimer.cpp
  Misc/PlanarDomain.cpp
  Misc/SphericalDomain.cpp
  Misc/Vector3D.cpp
  Misc/VtkTypeInfo.cpp
  Misc/XYRectangle.cpp
  )
IF(IMESH_DEFS OR IGEOM_DEFS OR IREL_DEFS)
  APPEND_SET(SOURCES Misc/MsqIBase.cpp Misc/MsqIBase.hpp)
ENDIF()


#
# src/ObjectiveFuntion
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ObjectiveFunction)

APPEND_SET(HEADERS
  ObjectiveFunction/CompositeOFAdd.hpp
  ObjectiveFunction/CompositeOFMultiply.hpp
  ObjectiveFunction/CompositeOFScalarAdd.hpp
  ObjectiveFunction/CompositeOFScalarMultiply.hpp
  ObjectiveFunction/LInfTemplate.hpp
  ObjectiveFunction/LPtoPTemplate.hpp
  ObjectiveFunction/MaxTemplate.hpp
  ObjectiveFunction/OFEvaluator.hpp
  ObjectiveFunction/ObjectiveFunction.hpp
  ObjectiveFunction/ObjectiveFunctionTemplate.hpp
  ObjectiveFunction/PatchPowerMeanP.hpp
  ObjectiveFunction/PMeanPTemplate.hpp
  ObjectiveFunction/StdDevTemplate.hpp
  ObjectiveFunction/VarianceTemplate.hpp
  )

APPEND_SET(SOURCES
  ObjectiveFunction/CompositeOFAdd.cpp
  ObjectiveFunction/CompositeOFMultiply.cpp
  ObjectiveFunction/CompositeOFScalarAdd.cpp
  ObjectiveFunction/CompositeOFScalarMultiply.cpp
  ObjectiveFunction/LInfTemplate.cpp
  ObjectiveFunction/LPtoPTemplate.cpp
  ObjectiveFunction/MaxTemplate.cpp
  ObjectiveFunction/OFEvaluator.cpp
  ObjectiveFunction/ObjectiveFunction.cpp
  ObjectiveFunction/ObjectiveFunctionTemplate.cpp
  ObjectiveFunction/PatchPowerMeanP.cpp
  ObjectiveFunction/PMeanPTemplate.cpp
  ObjectiveFunction/StdDevTemplate.cpp
  ObjectiveFunction/VarianceTemplate.cpp
  )

#
# src/Control/QualityAssessor
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityAssessor)

APPEND_SET(HEADERS
  QualityAssessor/QualityAssessor.hpp
  )

APPEND_SET(SOURCES
  QualityAssessor/QualityAssessor.cpp
  )

#
# src/Control/QualityImprover
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover)

APPEND_SET(HEADERS
  QualityImprover/PatchSetUser.hpp
  QualityImprover/QualityImprover.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/QualityImprover.cpp
  )

#
# src/Control/QualityImprover/TopologyModifier
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/TopolofyModifier)

APPEND_SET(HEADERS
  QualityImprover/TopologyModifier/TopologyModifier.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/TopologyModifier/TopologyModifier.cpp
  )

#
# src/Control/QualityImprover/VertexMover
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/MeanMidNodeMover.hpp
  QualityImprover/VertexMover/NullImprover.hpp
  QualityImprover/VertexMover/VertexMover.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/MeanMidNodeMover.cpp
  QualityImprover/VertexMover/VertexMover.cpp
  )

#
# src/Control/QualityImprover/VertexMover/ConjugateGradient
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/ConjugateGradient)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/ConjugateGradient/ConjugateGradient.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/ConjugateGradient/ConjugateGradient.cpp
  )

#
# src/Control/QualityImprover/VertexMover/Relaxation
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/Relaxation)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/Relaxation/RelaxationSmoother.hpp
  QualityImprover/VertexMover/Relaxation/LaplacianSmoother.hpp
  QualityImprover/VertexMover/Relaxation/SmartLaplacianSmoother.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/Relaxation/RelaxationSmoother.cpp
  QualityImprover/VertexMover/Relaxation/LaplacianSmoother.cpp
  QualityImprover/VertexMover/Relaxation/SmartLaplacianSmoother.cpp
  )

#
# src/Control/QualityImprover/VertexMover/SteepestDescent
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/SteepestDescent)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/SteepestDescent/SteepestDescent.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/SteepestDescent/SteepestDescent.cpp
  )

#
# src/Control/QualityImprover/VertexMover/NonSmoothDescent
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/NonSmoothDescent)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/NonSmoothDescent/NonSmoothDescent.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/NonSmoothDescent/NonSmoothDescent.cpp
  )

#
# src/Control/QualityImprover/VertexMover/FeasibleNewton
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/FeasibleNewton)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/FeasibleNewton/FeasibleNewton.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/FeasibleNewton/FeasibleNewton.cpp
  )

#
# src/Control/QualityImprover/VertexMover/Randomize
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/Randomize)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/Randomize/Randomize.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/Randomize/Randomize.cpp
  )

#
# src/Control/QualityImprover/VertexMover/TrustRegion
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/TrustRegion)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/TrustRegion/TrustRegion.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/TrustRegion/TrustRegion.cpp
  )

#
# src/Control/QualityImprover/VertexMover/QuasiNewton
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityImprover/VertexMover/QuasiNewton)

APPEND_SET(HEADERS
  QualityImprover/VertexMover/QuasiNewton/QuasiNewton.hpp
  )

APPEND_SET(SOURCES
  QualityImprover/VertexMover/QuasiNewton/QuasiNewton.cpp
  )

#
# src/Control/QualityMetric
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric)

APPEND_SET(HEADERS
  QualityMetric/AddQualityMetric.hpp
  QualityMetric/AveragingQM.hpp
  QualityMetric/EdgeQM.hpp
  QualityMetric/ElemSampleQM.hpp
  QualityMetric/ElementMaxQM.hpp
  QualityMetric/ElementPMeanP.hpp
  QualityMetric/ElementQM.hpp
  QualityMetric/MultiplyQualityMetric.hpp
  QualityMetric/NumericalQM.hpp
  QualityMetric/PMeanPMetric.hpp
  QualityMetric/PowerQualityMetric.hpp
  QualityMetric/QualityMetric.hpp
  QualityMetric/ScalarAddQualityMetric.hpp
  QualityMetric/ScalarMultiplyQualityMetric.hpp
  QualityMetric/VertexMaxQM.hpp
  QualityMetric/VertexPMeanP.hpp
  QualityMetric/VertexQM.hpp 
  )

APPEND_SET(SOURCES
  QualityMetric/AddQualityMetric.cpp
  QualityMetric/AveragingQM.cpp
  QualityMetric/EdgeQM.cpp
  QualityMetric/ElementMaxQM.cpp
  QualityMetric/ElementPMeanP.cpp
  QualityMetric/ElementQM.cpp
  QualityMetric/MultiplyQualityMetric.cpp
  QualityMetric/NumericalQM.cpp
  QualityMetric/PMeanPMetric.cpp
  QualityMetric/PowerQualityMetric.cpp
  QualityMetric/QualityMetric.cpp
  QualityMetric/ScalarAddQualityMetric.cpp
  QualityMetric/ScalarMultiplyQualityMetric.cpp
  QualityMetric/VertexMaxQM.cpp
  QualityMetric/VertexPMeanP.cpp
  QualityMetric/VertexQM.cpp 
  )

#
# src/Control/QualityMetric/Shape
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/Shape)

APPEND_SET(HEADERS
  QualityMetric/Shape/AspectRatioGammaQualityMetric.hpp
  QualityMetric/Shape/ConditionNumberFunctions.hpp
  QualityMetric/Shape/ConditionNumberQualityMetric.hpp
  QualityMetric/Shape/IdealWeightInverseMeanRatio.hpp
  QualityMetric/Shape/IdealWeightMeanRatio.hpp
  QualityMetric/Shape/MeanRatioFunctions.hpp
  QualityMetric/Shape/VertexConditionNumberQualityMetric.hpp
  )

APPEND_SET(SOURCES
  QualityMetric/Shape/AspectRatioGammaQualityMetric.cpp
  QualityMetric/Shape/ConditionNumberQualityMetric.cpp
  QualityMetric/Shape/IdealWeightInverseMeanRatio.cpp
  QualityMetric/Shape/IdealWeightMeanRatio.cpp
  QualityMetric/Shape/VertexConditionNumberQualityMetric.cpp
  )

#
# src/Control/QualityMetric/Smoothness
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/Smoothness)

APPEND_SET(HEADERS
  QualityMetric/Smoothness/EdgeLengthQualityMetric.hpp
  QualityMetric/Smoothness/EdgeLengthRangeQualityMetric.hpp
  )

APPEND_SET(SOURCES
  QualityMetric/Smoothness/EdgeLengthQualityMetric.cpp
  QualityMetric/Smoothness/EdgeLengthRangeQualityMetric.cpp
  )

#
# src/Control/QualityMetric/TMP
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/TMP)

APPEND_SET(HEADERS
  QualityMetric/TMP/AffineMapMetric.hpp
  QualityMetric/TMP/TMPQualityMetric.hpp
  QualityMetric/TMP/TMPDerivs.hpp
  )

APPEND_SET(SOURCES
  QualityMetric/TMP/AffineMapMetric.cpp
  QualityMetric/TMP/TargetMetricUtil.cpp
  QualityMetric/TMP/TargetMetricUtil.hpp
  QualityMetric/TMP/TargetMetricDimIndep.hpp
  QualityMetric/TMP/TMPQualityMetric.cpp 
  )

#
# src/Control/QualityMetric/TMP/2D
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/TMP/2D)

APPEND_SET(HEADERS
  QualityMetric/TMP/2D/TargetMetric2D.hpp
  QualityMetric/TMP/2D/InvTransBarrier2D.hpp
  QualityMetric/TMP/2D/TSquared2D.hpp
  QualityMetric/TMP/2D/Target2DScale.hpp
  QualityMetric/TMP/2D/Target2DShape.hpp
  QualityMetric/TMP/2D/Target2DShapeAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeBarrier.hpp
  QualityMetric/TMP/2D/Target2DShapeOrient.hpp
  QualityMetric/TMP/2D/Target2DShapeOrientAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeOrientBarrier.hpp
  QualityMetric/TMP/2D/Target2DShapeOrientBarrierAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeSize.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeAlt2.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrier.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrierAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrierAlt2.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrient.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientBarrier.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientBarrierAlt1.hpp
  QualityMetric/TMP/2D/Target2DShapeSizeUntangle.hpp
  QualityMetric/TMP/2D/Target2DSize.hpp
  QualityMetric/TMP/2D/Target2DSizeBarrier.hpp
  QualityMetric/TMP/2D/Target2DSizeUntangle.hpp
  QualityMetric/TMP/2D/Target2DSquared.hpp
  QualityMetric/TMP/2D/Target2DSum.hpp
  QualityMetric/TMP/2D/Target2DOffset.hpp
  QualityMetric/TMP/2D/Target2DTau.hpp
  QualityMetric/TMP/2D/Target2DUntangle.hpp
  QualityMetric/TMP/2D/Target2DUntangleAlt1.hpp
  QualityMetric/TMP/2D/InverseMeanRatio2D.hpp 
  )

APPEND_SET(SOURCES
  QualityMetric/TMP/2D/TargetMetric2D.cpp
  QualityMetric/TMP/2D/InvTransBarrier2D.cpp
  QualityMetric/TMP/2D/TSquared2D.cpp
  QualityMetric/TMP/2D/Target2DScale.cpp
  QualityMetric/TMP/2D/Target2DShape.cpp
  QualityMetric/TMP/2D/Target2DShapeAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeBarrier.cpp
  QualityMetric/TMP/2D/Target2DShapeOrient.cpp
  QualityMetric/TMP/2D/Target2DShapeOrientAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeOrientBarrier.cpp
  QualityMetric/TMP/2D/Target2DShapeOrientBarrierAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeSize.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeAlt2.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrier.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrierAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeBarrierAlt2.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrient.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientBarrier.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeOrientBarrierAlt1.cpp
  QualityMetric/TMP/2D/Target2DShapeSizeUntangle.cpp
  QualityMetric/TMP/2D/Target2DSize.cpp
  QualityMetric/TMP/2D/Target2DSizeBarrier.cpp
  QualityMetric/TMP/2D/Target2DSizeUntangle.cpp
  QualityMetric/TMP/2D/Target2DSquared.cpp
  QualityMetric/TMP/2D/Target2DSum.cpp
  QualityMetric/TMP/2D/Target2DOffset.cpp
  QualityMetric/TMP/2D/Target2DTau.cpp
  QualityMetric/TMP/2D/Target2DUntangle.cpp
  QualityMetric/TMP/2D/Target2DUntangleAlt1.cpp
  QualityMetric/TMP/2D/InverseMeanRatio2D.cpp 
  )


#
# src/Control/QualityMetric/TMP/3D
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/TMP/3D)

APPEND_SET(HEADERS
  QualityMetric/TMP/3D/InverseMeanRatio3D.hpp
  QualityMetric/TMP/3D/TargetMetric3D.hpp
  QualityMetric/TMP/3D/TSquared3D.hpp
  QualityMetric/TMP/3D/Target3DScale.hpp
  QualityMetric/TMP/3D/Target3DShape.hpp
  QualityMetric/TMP/3D/Target3DShapeBarrier.hpp
  QualityMetric/TMP/3D/Target3DShapeBarrierAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeOrient.hpp
  QualityMetric/TMP/3D/Target3DShapeOrientAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeOrientBarrier.hpp
  QualityMetric/TMP/3D/Target3DShapeOrientBarrierAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeSize.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrier.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt2.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt3.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrient.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientBarrier.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientBarrierAlt1.hpp
  QualityMetric/TMP/3D/Target3DShapeSizeUntangle.hpp
  QualityMetric/TMP/3D/Target3DSize.hpp
  QualityMetric/TMP/3D/Target3DSizeBarrier.hpp
  QualityMetric/TMP/3D/Target3DSizeUntangle.hpp
  QualityMetric/TMP/3D/Target3DSquared.hpp
  QualityMetric/TMP/3D/Target3DSum.hpp
  QualityMetric/TMP/3D/Target3DOffset.hpp
  QualityMetric/TMP/3D/Target3DTau.hpp
  QualityMetric/TMP/3D/Target3DUntangle.hpp 
  QualityMetric/TMP/3D/Target3DUntangleAlt1.hpp 
  )

APPEND_SET(SOURCES
  QualityMetric/TMP/3D/InverseMeanRatio3D.cpp
  QualityMetric/TMP/3D/TargetMetric3D.cpp
  QualityMetric/TMP/3D/TSquared3D.cpp
  QualityMetric/TMP/3D/Target3DScale.cpp
  QualityMetric/TMP/3D/Target3DShape.cpp
  QualityMetric/TMP/3D/Target3DShapeBarrier.cpp
  QualityMetric/TMP/3D/Target3DShapeBarrierAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeOrient.cpp
  QualityMetric/TMP/3D/Target3DShapeOrientAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeOrientBarrier.cpp
  QualityMetric/TMP/3D/Target3DShapeOrientBarrierAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeSize.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrier.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt2.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeBarrierAlt3.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrient.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientBarrier.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeOrientBarrierAlt1.cpp
  QualityMetric/TMP/3D/Target3DShapeSizeUntangle.cpp
  QualityMetric/TMP/3D/Target3DSize.cpp
  QualityMetric/TMP/3D/Target3DSizeBarrier.cpp
  QualityMetric/TMP/3D/Target3DSizeUntangle.cpp
  QualityMetric/TMP/3D/Target3DSquared.cpp
  QualityMetric/TMP/3D/Target3DSum.cpp
  QualityMetric/TMP/3D/Target3DOffset.cpp
  QualityMetric/TMP/3D/Target3DTau.cpp
  QualityMetric/TMP/3D/Target3DUntangle.cpp 
  QualityMetric/TMP/3D/Target3DUntangleAlt1.cpp 
  )

#
# src/Control/QualityMetric/Untangle
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/Untangle)

APPEND_SET(HEADERS
  QualityMetric/Untangle/UntangleBetaQualityMetric.hpp
  )

APPEND_SET(SOURCES
  QualityMetric/Untangle/UntangleBetaQualityMetric.cpp
  )

#
# src/Control/QualityMetric/Volume
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/QualityMetric/Volume)

APPEND_SET(HEADERS
  QualityMetric/Volume/EdgeLengthMetric.hpp
  QualityMetric/Volume/LocalSizeQualityMetric.hpp
  QualityMetric/Volume/SizeMetric.hpp
  )

APPEND_SET(SOURCES
  QualityMetric/Volume/EdgeLengthMetric.cpp
  QualityMetric/Volume/LocalSizeQualityMetric.cpp
  QualityMetric/Volume/SizeMetric.cpp
  )

#
# src/Control/QualityMetric/TargetCalculator
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/TargetCalculator)

APPEND_SET(HEADERS
  TargetCalculator/CachingTargetCalculator.hpp
  TargetCalculator/IdealShapeTarget.hpp
  TargetCalculator/InverseMetricWeight.hpp
  TargetCalculator/JacobianCalculator.hpp
  TargetCalculator/LambdaTarget.hpp
  TargetCalculator/LambdaConstant.hpp
  TargetCalculator/LVQDTargetCalculator.hpp
  TargetCalculator/MetricWeight.hpp
  TargetCalculator/RefMeshTargetCalculator.hpp
  TargetCalculator/ReferenceMesh.hpp
  TargetCalculator/RefSizeTargetCalculator.hpp
  TargetCalculator/RemainingWeight.hpp
  TargetCalculator/TargetCalculator.hpp
  TargetCalculator/TargetReader.hpp
  TargetCalculator/TargetWriter.hpp
  TargetCalculator/TetDihedralWeight.hpp
  TargetCalculator/WeightCalculator.hpp
  TargetCalculator/WeightReader.hpp 
  )

APPEND_SET(SOURCES
  TargetCalculator/CachingTargetCalculator.cpp
  TargetCalculator/IdealShapeTarget.cpp
  TargetCalculator/InverseMetricWeight.cpp
  TargetCalculator/JacobianCalculator.cpp
  TargetCalculator/LambdaTarget.cpp
  TargetCalculator/LambdaConstant.cpp
  TargetCalculator/LVQDTargetCalculator.cpp
  TargetCalculator/MetricWeight.cpp
  TargetCalculator/RefMeshTargetCalculator.cpp
  TargetCalculator/ReferenceMesh.cpp
  TargetCalculator/RefSizeTargetCalculator.cpp
  TargetCalculator/RemainingWeight.cpp
  TargetCalculator/TargetCalculator.cpp
  TargetCalculator/TargetReader.cpp
  TargetCalculator/TargetWriter.cpp
  TargetCalculator/TetDihedralWeight.cpp
  TargetCalculator/WeightCalculator.cpp
  TargetCalculator/WeightReader.cpp 
  )

#
# itaps
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../itaps)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../itaps)



#the key here is that the generated files have to be marked as generated using the SET_SOURCE_FILES_PROPERTIES
#command. 
#
# Generate Fortran name mangling headers
#
# ADD_CUSTOM_COMMAND( 
#   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMesh_protos.h
#   COMMAND ${CMAKE_COMMAND}
#   ARGS    -D input_file:STRING=${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iMesh.h 
#           -D output_file:STRING=${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMesh_protos.h 
#           -D prefix:STRING=iMesh 
#           -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ITAPSFortranMangling.cmake
#   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iMesh.h
#   COMMENT "Running custom command"
#   )
# SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMesh_protos.h PROPERTIES GENERATED true)
# 
# ADD_CUSTOM_COMMAND( 
#   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h
#   COMMAND ${CMAKE_COMMAND}
#   ARGS    -D input_file:STRING=${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iMeshP.h 
#           -D output_file:STRING=${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h 
#           -D prefix:STRING=iMeshP 
#           -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ITAPSFortranMangling.cmake
#   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iMeshP.h
#   COMMENT "Running custom command"
#   )
# SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h PROPERTIES GENERATED true)
# 
# ADD_CUSTOM_COMMAND( 
#   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iGeom_protos.h
#   COMMAND ${CMAKE_COMMAND}
#   ARGS    -D input_file:STRING=${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iGeom.h 
#           -D output_file:STRING=${CMAKE_CURRENT_BINARY_DIR}/../itaps/iGeom_protos.h 
#           -D prefix:STRING=iGeom 
#           -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ITAPSFortranMangling.cmake
#   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iGeom.h
#   COMMENT "Running custom command"
#   )
# SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h PROPERTIES GENERATED true)
# 
# ADD_CUSTOM_COMMAND( 
#   OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iRel_protos.h
#   COMMAND ${CMAKE_COMMAND}
#   ARGS    -D input_file:STRING=${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iRel.h 
#           -D output_file:STRING=${CMAKE_CURRENT_BINARY_DIR}/../itaps/iRel_protos.h 
#           -D prefix:STRING=iRel 
#           -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/ITAPSFortranMangling.cmake
#   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../itaps/iRel.h
#   COMMENT "Running custom command"
#   )
# SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h PROPERTIES GENERATED true)
# 
# 
#APPEND_SET(HEADERS
#  ../itaps/iBase.h
#  ../itaps/iBase_FCDefs.h
#  ../itaps/iMesh.h
#  ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMesh_protos.h
#  ../itaps/iMeshP.h
#  ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iMeshP_protos.h
#  ../itaps/iGeom.h
#  ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iGeom_protos.h
#  ../itaps/iRel.h
#  ${CMAKE_CURRENT_BINARY_DIR}/../itaps/iRel_protos.h 
#  )


#
# Create the Mesquite_all_headers.hpp file.
#

SET( all_file ${CMAKE_CURRENT_BINARY_DIR}/../include/Mesquite_all_headers.hpp )
# Cannot do this correctly because list if all headers is too long
# of a command-line argument for some platforms (e.g. Windows.)
# Instead, use less optimial solution (doesn't automatically update.)
#
#ADD_CUSTOM_COMMAND( 
#  OUTPUT ${all_file}
#  COMMAND ${CMAKE_COMMAND}
#  ARGS -D output_file:STRING=${all_file}
#       -D headers:STRING="${HEADERS}"
#       -P ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/GenerateAllHeaders.cmake
#  DEPENDS ${HEADERS}
#  COMMENT "Generating Mesquite_all_headers.hpp" )
#
SET_SOURCE_FILES_PROPERTIES(${all_file} PROPERTIES GENERATED true)
#
file( WRITE ${all_file} "#ifndef MESQUITE_ALL_HEADERS_HPP\n#define MESQUITE_ALL_HEADERS_HPP\n" )
foreach( header ${HEADERS} )
  string( REGEX MATCH "[^/]+\\.h(pp)?" file ${header} )
  file( APPEND ${all_file} "#include \"${file}\"\n")
endforeach()
file( APPEND ${all_file} "#endif\n" )
  


#last thing to do is add then newly created header
APPEND_SET(HEADERS ${all_file} )

#
# C) Define the targets for package's library(s)
#

PACKAGE_ADD_LIBRARY(
  mesquite
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )

MACRO(APPEND_DEFINE VARNAME OPT_FLAG)
  IF (${OPT_FLAG})
    SET(${VARNAME} "${${VARNAME}} -D${OPT_FLAG}")
  ENDIF()
ENDMACRO()

# s/MESQUITE_DLL_EXPORTS/MESQUITE_STATIC_LIB/ if building static library
IF(${PACKAGE_NAME}_ENABLE_SHARED)
  SET( MESQUITE_LIB_DEFINES "${MESQUITE_DEFINES} -DMESQUITE_DLL_EXPORTS" )
ELSE()
  SET( MESQUITE_LIB_DEFINES "${MESQUITE_DEFINES} -DMESQUITE_STATIC_LIB" )
ENDIF()
SET_SOURCE_FILES_PROPERTIES( ${SOURCES} COMPILE_FLAGS ${MESQUITE_LIB_DEFINES} )


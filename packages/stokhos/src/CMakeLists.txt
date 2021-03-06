
INCLUDE(PackageLibraryMacros)

#
# A) Package-specific configuration options
#

SET(HAVE_STOKHOS_DAKOTA ${${PACKAGE_NAME}_ENABLE_TriKota})

PACKAGE_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)

#
# B) Define the header and source files (and directories)
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# Add path to Dakota headers in installed directory
IF (${PACKAGE_NAME}_ENABLE_TriKota)
INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}/../../TriKota/Dakota/install/include
  )
ENDIF()


SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.h
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

SET(HEADERS ${HEADERS}
  Stokhos.hpp
  Stokhos_ConfigDefs.h
  Stokhos_Version.hpp
  Stokhos_gaussq.h
  Stokhos_DynamicArrayTraits.hpp
  Stokhos_OneDOrthogPolyBasis.hpp
  Stokhos_RecurrenceBasis.hpp
  Stokhos_RecurrenceBasisImp.hpp
  Stokhos_HermiteBasis.hpp
  Stokhos_HermiteBasisImp.hpp
  Stokhos_JacobiBasis.hpp
  Stokhos_JacobiBasisImp.hpp
  Stokhos_DiscretizedStieltjesBasis.hpp
  Stokhos_DiscretizedStieltjesBasisImp.hpp
  Stokhos_RysBasis.hpp
  Stokhos_LegendreBasis.hpp
  Stokhos_LegendreBasisImp.hpp
  Stokhos_ClenshawCurtisLegendreBasis.hpp
  Stokhos_ClenshawCurtisLegendreBasisImp.hpp
  Stokhos_StieltjesPCEBasis.hpp
  Stokhos_StieltjesPCEBasisImp.hpp
  Stokhos_PecosOneDOrthogPolyBasis.hpp
  Stokhos_PecosOneDOrthogPolyBasisImp.hpp
  Stokhos_OrthogPolyBasis.hpp
  Stokhos_ProductBasis.hpp
  Stokhos_DerivBasis.hpp
  Stokhos_CompletePolynomialBasis.hpp
  Stokhos_CompletePolynomialBasisImp.hpp
  Stokhos_GramSchmidtBasis.hpp
  Stokhos_GramSchmidtBasisImp.hpp
  Stokhos_BasisFactory.hpp
  Stokhos_BasisFactoryImp.hpp
  Stokhos_Sparse3Tensor.hpp
  Stokhos_Sparse3TensorImp.hpp
  Stokhos_Dense3Tensor.hpp
  Stokhos_Dense3TensorImp.hpp
  Stokhos_Sparse3TensorUtilities.hpp
  Stokhos_Quadrature.hpp
  Stokhos_TensorProductQuadrature.hpp
  Stokhos_TensorProductQuadratureImp.hpp
  Stokhos_SparseGridQuadrature.hpp
  Stokhos_SparseGridQuadratureImp.hpp
  Stokhos_AnisoSparseGridQuadrature.hpp
  Stokhos_AnisoSparseGridQuadratureImp.hpp
  Stokhos_UserDefinedQuadrature.hpp
  Stokhos_UserDefinedQuadratureImp.hpp
  Stokhos_QuadratureFactory.hpp
  Stokhos_QuadratureFactoryImp.hpp
  Stokhos_OrthogPolyApprox.hpp
  Stokhos_OrthogPolyApproxImp.hpp
  Stokhos_OrthogPolyExpansion.hpp
  Stokhos_ConstantOrthogPolyExpansion.hpp
  Stokhos_ConstantOrthogPolyExpansionImp.hpp
  Stokhos_AlgebraicOrthogPolyExpansion.hpp
  Stokhos_AlgebraicOrthogPolyExpansionImp.hpp
  Stokhos_QuadOrthogPolyExpansion.hpp
  Stokhos_QuadOrthogPolyExpansionImp.hpp
  Stokhos_DerivOrthogPolyExpansion.hpp
  Stokhos_DerivOrthogPolyExpansionImp.hpp
  Stokhos_ForUQTKOrthogPolyExpansion.hpp
  Stokhos_ForUQTKOrthogPolyExpansionImp.hpp
  Stokhos_ExpansionFactory.hpp
  Stokhos_ExpansionFactoryImp.hpp
  Stokhos_StieltjesGramSchmidtBuilder.hpp
  Stokhos_StieltjesGramSchmidtBuilderImp.hpp
  Stokhos_MatrixFreeEpetraOp.hpp
  Stokhos_KLMatrixFreeEpetraOp.hpp
  Stokhos_MeanEpetraOp.hpp
  Stokhos_PreconditionerFactory.hpp
  Stokhos_IfpackPreconditionerFactory.hpp
  Stokhos_MLPreconditionerFactory.hpp
  Stokhos_MLPrecOp.hpp
  Stokhos_SGModelEvaluator.hpp
  Stokhos_SGQuadModelEvaluator.hpp
  Stokhos_SGInverseModelEvaluator.hpp
  Stokhos_VectorOrthogPoly.hpp
  Stokhos_VectorOrthogPolyImp.hpp
  Stokhos_VectorOrthogPolyTraitsEpetra.hpp
  Stokhos_EpetraVectorOrthogPoly.hpp
  Stokhos_EpetraMultiVectorOrthogPoly.hpp
  Stokhos_PCECovarianceOp.hpp
  Stokhos_PCEAnasaziKL.hpp
  Stokhos_KL_OneDEigenPair.hpp
  Stokhos_KL_ProductEigenPair.hpp
  Stokhos_KL_OneDExponentialCovarianceFunction.hpp
  Stokhos_KL_OneDExponentialCovarianceFunctionImp.hpp
  Stokhos_KL_ExponentialRandomField.hpp
  Stokhos_KL_ExponentialRandomFieldImp.hpp
  Stokhos_KLReducedMatrixFreeEpetraOp.hpp
  Stokhos_EpetraMultiVectorOperator.hpp
  )

SET(SOURCES ${SOURCES}
  Stokhos_Version.cpp
  Stokhos_SparseGridQuadrature.cpp
  Stokhos_AnisoSparseGridQuadrature.cpp
  Stokhos_MatrixFreeEpetraOp.cpp
  Stokhos_KLMatrixFreeEpetraOp.cpp
  Stokhos_MeanEpetraOp.cpp
  Stokhos_IfpackPreconditionerFactory.cpp
  Stokhos_MLPreconditionerFactory.cpp
  Stokhos_MLPrecOp.cpp
  Stokhos_EpetraVectorOrthogPoly.cpp
  Stokhos_EpetraMultiVectorOrthogPoly.cpp
  Stokhos_SGModelEvaluator.cpp
  Stokhos_SGQuadModelEvaluator.cpp
  Stokhos_SGInverseModelEvaluator.cpp
  Stokhos_PCECovarianceOp.cpp
  Stokhos_PCEAnasaziKL.cpp
  Stokhos_KLReducedMatrixFreeEpetraOp.cpp
  Stokhos_EpetraMultiVectorOperator.cpp
  )

# Sacado
IF (Stokhos_ENABLE_Sacado)
  INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/sacado)
  SET(HEADERS ${HEADERS}
    sacado/Sacado_PCE_OrthogPoly.hpp
    sacado/Sacado_PCE_OrthogPolyImp.hpp
    sacado/Sacado_PCE_OrthogPolyTraits.hpp
    )
ENDIF()

#
# C) Define the targets for package's library(s)
#

PACKAGE_ADD_LIBRARY(
  stokhos
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )

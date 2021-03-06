
INCLUDE(PackageLibraryMacros)

#
# A) Package-specific configuration options
#

PACKAGE_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)

#
# B) Define the header and source files (and directories)
#

#
# src
#

SET(HEADERS "")
SET(SOURCES "")
SET(KOKKOS_LINALG_CUDALIBS "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.h
  )

#
# Core Files
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# general headers, sources
APPEND_SET(HEADERS
    # standard
    Kokkos_LinAlgVersion.hpp
    # data structures
    Kokkos_CrsGraph.hpp
    Kokkos_CrsMatrix.hpp
    Kokkos_MultiVector.hpp
    Kokkos_VbrMatrix.hpp
    # default template parameters
    Kokkos_DefaultKernels.hpp
    # kernels
    Kokkos_DefaultBlockSparseKernelOps.hpp
    Kokkos_DefaultSparseMultiplyKernelOps.hpp
    Kokkos_DefaultSparseSolveKernelOps.hpp
    Kokkos_DefaultRelaxationKernelOps.hpp
    Kokkos_MultiVectorKernelOps.hpp
    # traits
    Kokkos_DefaultArithmetic.hpp
    Kokkos_DefaultBlockSparseOps.hpp
    Kokkos_DefaultSparseOps.hpp
    Kokkos_DefaultRelaxation.hpp
  )

APPEND_SET(SOURCES
    Kokkos_LinAlgVersion.cpp
  )

SET(KOKKOS_LINALG_CUDALIBS "")
ASSERT_DEFINED(Kokkos_ENABLE_Thrust)
IF (Kokkos_ENABLE_Thrust)
  # this pulls in Thrust, CUSP, and Kokkos/NodeAPI
  include_directories(${Kokkos_INCLUDE_DIRS})
  CUDA_ADD_LIBRARY(kokkoslinalg_cuda Kokkos_DefaultArithmetic.cu Kokkos_DefaultBlockSparseOps.cu Kokkos_DefaultSparseOps.cu Kokkos_DefaultRelaxation.cu)
  APPEND_SET(KOKKOS_LINALG_CUDALIBS "kokkoslinalg_cuda")
  TARGET_LINK_LIBRARIES(kokkoslinalg_cuda teuchos)
ENDIF()

#
# C) Define the targets for package's library(s)
#

PACKAGE_ADD_LIBRARY(
  kokkoslinalg
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  DEPLIBS kokkos kokkosnodeapi ${KOKKOS_LINALG_CUDALIBS}
  )

PACKAGE_ADD_TEST_DIRECTORIES(examples)
PACKAGE_ADD_TEST_DIRECTORIES(test)

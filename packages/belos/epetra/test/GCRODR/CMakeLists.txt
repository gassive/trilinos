
INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)

ASSERT_DEFINED(${PACKAGE_NAME}_ENABLE_EpetraExt)
ASSERT_DEFINED(${PACKAGE_NAME}_ENABLE_Triutils)
IF (${PACKAGE_NAME}_ENABLE_Triutils AND ${PACKAGE_NAME}_ENABLE_EpetraExt)
    
  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    gcrodr_hb
    SOURCES test_gcrodr_hb.cpp createEpetraProblem.cpp
    COMM serial mpi
    ARGS
      "--debug --verbose --filename=sherman5.hb --tol=1e-4 --num-rhs=2 --max-subspace=61 --recycle=23 --max-cycles=75"
    STANDARD_PASS_OUTPUT 
    )

  PACKAGE_COPY_FILES_TO_BINARY_DIR(CopyTestGCRODRFiles
    SOURCE_DIR ${CMAKE_SOURCE_DIR}/packages/belos/epetra/example/GCRODR
    SOURCE_FILES sherman5.hb
    EXEDEPS gcrodr_hb
    )

  ASSERT_DEFINED(${PACKAGE_NAME}_ENABLE_Ifpack)
  IF(${PACKAGE_NAME}_ENABLE_Ifpack)

    PACKAGE_ADD_EXECUTABLE_AND_TEST(
      prec_gcrodr_hb
      SOURCES test_prec_gcrodr_hb.cpp createEpetraProblem.cpp
      COMM serial mpi
      ARGS
        "--debug --verbose --filename=sherman5.hb --tol=1e-4 --num-rhs=2 --max-subspace=61 --recycle=23 --max-cycles=75"
        "--debug --verbose --filename=sherman5.hb --tol=1e-10 --num-rhs=2 --max-subspace=10 --recycle=2"
      STANDARD_PASS_OUTPUT 
      )

  ENDIF(${PACKAGE_NAME}_ENABLE_Ifpack)

ENDIF()

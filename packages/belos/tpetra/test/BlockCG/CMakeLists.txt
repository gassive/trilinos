
INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)

PACKAGE_ADD_EXECUTABLE_AND_TEST(
  Tpetra_BlockCG_hb_test
  SOURCES test_bl_cg_hb.cpp 
  ARGS 
  COMM serial mpi
  DEPLIBS tpetra
  )

#PACKAGE_ADD_EXECUTABLE_AND_TEST(
#  Tpetra_BlockCG_complex_hb_test
#  SOURCES test_bl_cg_complex_hb.cpp 
#  ARGS 
#  COMM serial mpi
#  )

# PACKAGE_ADD_EXECUTABLE_AND_TEST(
#   Tpetra_BlockCG_hb_multiprec_test
#   SOURCES test_bl_cg_hb_multiprec.cpp 
#   ARGS 
#   COMM serial mpi
#   )

#PACKAGE_COPY_FILES_TO_BINARY_DIR(Tpetra_BlockCG_complex_hb_CopyFiles
#  SOURCE_DIR ${CMAKE_SOURCE_DIR}/packages/belos/test/MVOPTester
#  SOURCE_FILES mhd1280b.cua
#  EXEDEPS Tpetra_BlockCG_complex_hb_test
#  )

PACKAGE_COPY_FILES_TO_BINARY_DIR(Tpetra_BlockCG_hb_CopyFiles1
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/packages/belos/tpetra/test/BlockCG
  SOURCE_FILES bcsstk17.rsa bcsstk18.rsa
  EXEDEPS Tpetra_BlockCG_hb_test 
  # EXEDEPS Tpetra_BlockCG_hb_test Tpetra_BlockCG_hb_multiprec_test
  )

PACKAGE_COPY_FILES_TO_BINARY_DIR(Tpetra_BlockCG_hb_CopyFiles2
  SOURCE_DIR ${CMAKE_SOURCE_DIR}/packages/belos/epetra/example/BlockCG
  SOURCE_FILES bcsstk14.hb bcsstk15.hb
  EXEDEPS Tpetra_BlockCG_hb_test 
  # EXEDEPS Tpetra_BlockCG_hb_test Tpetra_BlockCG_hb_multiprec_test
  )


INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)
INCLUDE(MultilineSet)

ASSERT_DEFINED(PACKAGE_SOURCE_DIR CMAKE_CURRENT_SOURCE_DIR)

FILE(GLOB SOURCES *.cpp)

PACKAGE_ADD_EXECUTABLE(
  vdp
  SOURCES ${SOURCES}
  DEPLIBS rythmos_test_models
  COMM serial mpi  
  )

SET(STANDARD_TEST_CONFIG
  COMM serial mpi  
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
  )

PACKAGE_ADD_TEST(
  vdp
  NAME VanDerPolOscillator_ETP
  ARGS "--verbose"
  ${STANDARD_TEST_CONFIG}
  )


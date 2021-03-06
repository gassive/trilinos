
INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)

PACKAGE_ADD_EXECUTABLE_AND_TEST(
  ParameterList_test 
  SOURCES cxx_main.cpp 
  ARGS --verbose
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
  )

PACKAGE_ADD_EXECUTABLE_AND_TEST(
  ParameterList_UnitTest
  SOURCES ParameterList_UnitTest.cpp Teuchos_StandardUnitTestMain.cpp
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
  )

PACKAGE_ADD_EXECUTABLE_AND_TEST(
  ParameterList_UnitTest_Parallel
  SOURCES ParameterList_UnitTest_Parallel.cpp Teuchos_StandardUnitTestMain.cpp
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

PACKAGE_ADD_TEST(
  ParameterList_UnitTest_Parallel
  NAME_POSTFIX one
  NUM_MPI_PROCS 1
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  )

PACKAGE_COPY_FILES_TO_BINARY_DIR(CopyPLXmlFiles
  DEST_FILES input.xml
  SOURCE_PREFIX "_"
  EXEDEPS ParameterList_UnitTest_Parallel
  )


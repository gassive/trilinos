INCLUDE(PackageAddExecutableAndTest)


PACKAGE_ADD_EXECUTABLE_AND_TEST(
  supportUnitTests
  SOURCES
    validate_apply_op_UnitTests.cpp
    supportUnitTestsHelpers.cpp
    ${PACKAGE_SOURCE_DIR}/../teuchos/test/UnitTest/Teuchos_StandardUnitTestMain.cpp
  ARGS " "
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
  )


PACKAGE_ADD_EXECUTABLE_AND_TEST(
  RTOpSubRangeDecoratorUnitTests
  SOURCES
    RTOpSubRangeDecorator_UnitTests.cpp
    ${PACKAGE_SOURCE_DIR}/../teuchos/test/UnitTest/Teuchos_StandardUnitTestMain.cpp
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
  )


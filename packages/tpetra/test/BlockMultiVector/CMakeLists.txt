
INCLUDE(PackageAddExecutableAndTest)


PACKAGE_ADD_EXECUTABLE_AND_TEST(
  BlockMultiVector_UnitTests
  SOURCES
    BlockMultiVector_UnitTests
    ${PACKAGE_SOURCE_DIR}/../teuchos/test/UnitTest/Teuchos_StandardUnitTestMain.cpp
  COMM serial mpi
  STANDARD_PASS_OUTPUT
  DEPLIBS kokkos kokkoslinalg kokkosnodeapi
  )

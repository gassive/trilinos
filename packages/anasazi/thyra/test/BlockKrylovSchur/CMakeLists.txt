
INCLUDE(PackageAddExecutableAndTest)

IF(Anasazi_ENABLE_Epetra)
  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    BlockKrylovSchurThyra_test
    SOURCES cxx_main.cpp
    ARGS --verbose
    COMM serial mpi
    STANDARD_PASS_OUTPUT
    )
ENDIF()

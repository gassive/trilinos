
INCLUDE(PackageAddExecutableAndTest)

IF (${PACKAGE_NAME}_ENABLE_COMPLEX)

  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    DenseMatrix_test  
    SOURCES cxx_main.cpp
    NUM_MPI_PROCS 1
    COMM serial mpi
    ARGS -v
    STANDARD_PASS_OUTPUT
    )

  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    SymDenseMatrix_test
    SOURCES cxx_main_sym.cpp
    NUM_MPI_PROCS 1
    COMM serial mpi
    ARGS -v
    STANDARD_PASS_OUTPUT
    )

ENDIF()

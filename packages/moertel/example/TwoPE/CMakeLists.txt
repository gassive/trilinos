
INCLUDE(PackageAddExecutableAndTest)


PACKAGE_ADD_EXECUTABLE_AND_TEST(
  test1
  SOURCES test1.cpp
  ARGS 
  COMM mpi
  NUM_MPI_PROCS 2
  PASS_REGULAR_EXPRESSION "Test passed!"
  )

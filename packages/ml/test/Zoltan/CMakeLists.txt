
INCLUDE(PackageAddExecutableAndTest)

PACKAGE_ADD_EXECUTABLE_AND_TEST(
  Zoltan
  SOURCES cxx_main.cpp
  NUM_MPI_PROCS 1-4
  COMM mpi
  )

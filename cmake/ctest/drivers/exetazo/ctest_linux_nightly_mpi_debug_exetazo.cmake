
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.exetazo.gcc.cmake")

#
# Set the options specific to this build case
#

SET(COMM_TYPE MPI)
SET(BUILD_TYPE DEBUG)
SET(BUILD_DIR_NAME MPI_DEBUG)
#SET(CTEST_TEST_TYPE CONTINUOUS)
SET(CTEST_TEST_TYPE EXPERIMENTAL)

#SET(CTEST_DO_COVERAGE_TESTING TRUE)
#SET(CTEST_DO_MEMORY_TESTING TRUE)

SET( EXTRA_CONFIGURE_OPTIONS
  "-DDART_TESTING_TIMEOUT:STRING=150"
  "-DTrilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=ON"
  "-DBUILD_SHARED_LIBS:BOOL=ON"
  )
#  "-DTPL_ENABLE_ParMETIS:BOOL=ON"
#  "-DParMETIS_LIBRARY_DIRS:PATH=/home/kddevin/code/ParMETIS3_1"
#  "-DTPL_ENABLE_Scotch:BOOL=ON"
#  "-DScotch_INCLUDE_DIRS:PATH=/home/kddevin/code/scotch_5.1/include"
#  "-DScotch_LIBRARY_DIRS:PATH=/home/kddevin/code/scotch_5.1/lib"
#  )

#
# Set the rest of the system-specific options and run the dashboard build/test
#

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()

  
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/../../TrilinosCTestDriverCore.cmake")

#
# Platform/compiler specific options for godel using gcc
#

MACRO(TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER)

  # Base of Trilinos/cmake/ctest then BUILD_DIR_NAME
  SET( CTEST_DASHBOARD_ROOT "${TRILINOS_CMAKE_DIR}/../../${BUILD_DIR_NAME}" )

  SET( CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}" )
  
  SET( CTEST_BUILD_FLAGS "-j8 -i" )

  SET_DEFAULT( CTEST_PARALLEL_LEVEL "8" )

  SET( CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind )
  #SET( CTEST_MEMORYCHECK_COMMAND_OPTIONS )

  SET_DEFAULT( Trilinos_ENABLE_SECONDARY_STABLE_CODE ON)

  # Only turn on PyTrilinos for shared libraries
  SET_DEFAULT( Trilinos_EXCLUDE_PACKAGES ${EXTRA_EXCLUDE_PACKAGES} PyTrilinos TriKota Optika)
  
  SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
    "-DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}"
    "-DMEMORYCHECK_COMMAND:FILEPATH=/usr/bin/valgrind"
    "-DTrilinos_ENABLE_TriKota:BOOL=OFF"
    )

  SET_DEFAULT(COMPILER_VERSION "ICPC-11.0.074")
  
  IF (COMM_TYPE STREQUAL MPI)

    #MESSAGE(FATAL_ERROR "Error, Intel build does not support MPI yet!")
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
      ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
      "-DTPL_ENABLE_MPI:BOOL=ON"
      "-DMPI_BASE_DIR:PATH=/usr/local/openmpi-icc11"
      "-DCMAKE_LIBRARY_PATH:PATH=/usr/lib64"
      "-DNetcdf_LIBRARY_DIRS=/home/bmpersc/lib/icpc-11.0/netcdf-4.0/lib"
      "-DNetcdf_INCLUDE_DIRS=/home/bmpersc/lib/icpc-11.0/netcdf-4.0/include"
      "-DTPL_LAPACK_LIBRARIES:STRING=-L/opt/intel/11.0.074/mkl/lib/em64t/ -lmkl_lapack"
      "-DTPL_BLAS_LIBRARIES:STRING=-L/opt/intel/11.0.074/mkl/lib/em64t -L/opt/intel/11.0.074/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5"
      )
 
  ELSE()
  
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
      ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
      "-DCMAKE_C_COMPILER:FILEPATH=/opt/intel/11.0.074/bin/intel64/icc"
      "-DCMAKE_CXX_COMPILER:FILEPATH=/opt/intel/11.0.074/bin/intel64/icpc"
      "-DCMAKE_Fortran_COMPILER:FILEPATH=/opt/intel/11.0.074/bin/intel64/ifort"
      "-DCMAKE_CXX_FLAGS:STRING=-diag-disable 597"
      "-DCMAKE_LIBRARY_PATH:PATH=/usr/lib64"
      "-DNetcdf_LIBRARY_DIRS=/home/bmpersc/lib/icpc-11.0/netcdf-4.0/lib"
      "-DNetcdf_INCLUDE_DIRS=/home/bmpersc/lib/icpc-11.0/netcdf-4.0/include"
      "-DTPL_LAPACK_LIBRARIES:STRING=-L/opt/intel/11.0.074/mkl/lib/em64t/ -lmkl_lapack"
      "-DTPL_BLAS_LIBRARIES:STRING=-L/opt/intel/11.0.074/mkl/lib/em64t -L/opt/intel/11.0.074/mkl/lib/intel64 -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread"
      )
  
  ENDIF()

  TRILINOS_CTEST_DRIVER()

ENDMACRO()

  
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/../../TrilinosCTestDriverCore.cmake")

#
# Platform/compiler specific options for godel using gcc
#

MACRO(TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER)

  # Base of Trilinos/cmake/ctest then BUILD_DIR_NAME
  SET( CTEST_DASHBOARD_ROOT "${TRILINOS_CMAKE_DIR}/../../${BUILD_DIR_NAME}" )

  SET( CTEST_NOTES_FILES "${CTEST_SCRIPT_DIRECTORY}/${CTEST_SCRIPT_NAME}" )
  
  SET( CTEST_BUILD_FLAGS "-j8 -i" )

  SET( CTEST_MEMORYCHECK_COMMAND /usr/bin/valgrind )
  #SET( CTEST_MEMORYCHECK_COMMAND_OPTIONS )

  # Only turn on PyTrilinos for shared libraries
  SET_DEFAULT( Trilinos_EXCLUDE_PACKAGES PyTrilinos TriKota Optika)
  
  SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
    "-DCMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}"
    "-DMEMORYCHECK_COMMAND:FILEPATH=/usr/bin/valgrind"
    "-DTrilinos_ENABLE_TriKota:BOOL=OFF"
    )

  SET_DEFAULT(COMPILER_VERSION "ICPC-10.1.0.15")
  
  IF (COMM_TYPE STREQUAL MPI)

    MESSAGE(FATAL_ERROR "Error, Intel build does not support MPI yet!")
  
  ELSE()
  
    SET( EXTRA_SYSTEM_CONFIGURE_OPTIONS
      ${EXTRA_SYSTEM_CONFIGURE_OPTIONS}
      "-DCMAKE_C_COMPILER:FILEPATH=/opt/intel/cc/10.1.015/bin/icc"
      "-DCMAKE_CXX_COMPILER:FILEPATH=/opt/intel/cc/10.1.015/bin/icpc"
      "-DCMAKE_Fortran_COMPILER:FILEPATH=/usr/bin/f77"
      "-DCMAKE_CXX_FLAGS:STRING=-diag-disable 597"
      "-DCMAKE_LIBRARY_PATH:PATH=/usr/lib64"
      )
  
  ENDIF()

  TRILINOS_CTEST_DRIVER()

ENDMACRO()

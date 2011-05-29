
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.trilinos-test2.gcc.cmake")

#
# Set the options specific to this build case
#

SET(COMM_TYPE SERIAL)
SET(BUILD_TYPE DEBUG)
SET(BUILD_DIR_NAME SERIAL_DEBUG_DEV)

SET(Trilinos_ENABLE_SECONDARY_STABLE_CODE ON)

SET( EXTRA_CONFIGURE_OPTIONS
  "-DDART_TESTING_TIMEOUT:STRING=120"
  "-DTrilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=ON"
  "-DTrilinos_DATA_DIR:STRING=$ENV{TRILINOSDATADIRECTORY}"
  "-DTPL_ENABLE_Pthread:BOOL=ON"
  "-DTPL_ENABLE_Boost:BOOL=ON"
  "-DBoost_INCLUDE_DIRS:FILEPATH=/home/trilinos/include"
  "-DTPL_ENABLE_ExodusII:BOOL=ON"
  "-DTPL_ENABLE_Nemesis:BOOL=ON"
  "-DNemesis_INCLUDE_DIRS:FILEPATH=/home/trilinos/tpl/gcc4.1.2/nemesis_3.09/include"
  "-DNemesis_LIBRARY_DIRS:FILEPATH=/home/trilinos/tpl/gcc4.1.2/nemesis_3.09/lib"
  "-DNOX_ENABLE_ABSTRACT_IMPLEMENTATION_LAPACK=ON"
  )

#
# Set the rest of the system-specific options and run the dashboard build/test
#

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()

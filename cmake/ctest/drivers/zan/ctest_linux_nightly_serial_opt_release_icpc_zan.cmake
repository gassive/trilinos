
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.zan.icpc.cmake")
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/../../../TrilinosVersion.cmake")

#
# Set the options specific to this build case
#

SET(COMM_TYPE SERIAL)
SET(BUILD_TYPE RELEASE)
SET(BUILD_DIR_NAME "SERIAL_OPT_RELEASE_ICPC_${Trilinos_VERSION}")
SET(Trilinos_TRACK ${Trilinos_TESTING_TRACK})
SET(Trilinos_BRANCH ${Trilinos_REPOSITORY_BRANCH})
SET(CTEST_TEST_TYPE Experimental)

#SET(Trilinos_PACKAGES Teuchos RTOp GlobiPack Thyra OptiPack Stratimikos Phalanx Rythmos)
SET(EXTRA_EXCLUDE_PACKAGES Didasko Sundance Piro Rythmos TrilinosCouplings NOX STK Pamgen Thyra Tpetra Zoltan Stokhos FEApp)

SET( EXTRA_CONFIGURE_OPTIONS
  "-DDART_TESTING_TIMEOUT:STRING=120"
  "-DTrilinos_ENABLE_Fortran:BOOL=ON"
  "-DTPL_ENABLE_Boost:BOOL=ON"
  )

#
# Set the rest of the system-specific options and run the dashboard build/test
#

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()

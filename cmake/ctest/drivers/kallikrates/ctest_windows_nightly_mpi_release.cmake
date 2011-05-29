INCLUDE("${CTEST_SCRIPT_DIRECTORY}/TrilinosCTestDriverCore.kallikrates.msvc.cmake")
INCLUDE("${CTEST_SCRIPT_DIRECTORY}/../../../TrilinosVersion.cmake")

#
# Set the options specific to this build case
#
#SET(CTEST_DO_UPDATES FALSE)
SET(COMM_TYPE MPI)
SET(BUILD_TYPE RELEASE)
SET(BUILD_DIR_NAME "${COMM_TYPE}_${BUILD_TYPE}_${Trilinos_VERSION}")
SET(Trilinos_TRACK ${Trilinos_TESTING_TRACK})

SET(Trilinos_BRANCH ${Trilinos_REPOSITORY_BRANCH})

#
# Set the rest of the system-specific options and run the dashboard build/test
#

TRILINOS_SYSTEM_SPECIFIC_CTEST_DRIVER()

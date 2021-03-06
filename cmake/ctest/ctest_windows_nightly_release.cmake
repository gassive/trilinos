#------------------------------------------------
# ejguill
# Nightly WINDOWS tests
# Release
#------------------------------------------------
SET (CTEST_SOURCE_NAME Trilinos)
SET (TEST_TYPE nightly)
SET (BUILD_TYPE release)

SET (CTEST_DASHBOARD_ROOT "C:/dashboards")
SET (CTEST_CMAKE_COMMAND "\"${CMAKE_EXECUTABLE_NAME}\"")

# Options for Nightly builds
SET (CTEST_BACKUP_AND_RESTORE TRUE)
SET (CTEST_START_WITH_EMPTY_BINARY_DIRECTORY TRUE)


SET (CTEST_CVS_COMMAND
  "C:/Program Files/CVSNT/cvs.exe"
)

SET (CTEST_CVS_CHECKOUT
  "cvs -Q -d :ext:$ENV{USER}@software.sandia.gov:/space/CVS co ${CTEST_SOURCE_NAME}"
)


SET (CTEST_BINARY_NAME ${CTEST_SOURCE_NAME}-Build)
SET (CTEST_SOURCE_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_SOURCE_NAME}")
SET (CTEST_BINARY_DIRECTORY "${CTEST_DASHBOARD_ROOT}/${CTEST_BINARY_NAME}")

SET (CTEST_COMMAND 
  #"\"${CTEST_EXECUTABLE_NAME}\" -D NightlyStart"
  #"\"${CTEST_EXECUTABLE_NAME}\" -D NightlyConfigure"
  "\"${CTEST_EXECUTABLE_NAME}\" -D NightlyBuild"
  "\"${CTEST_EXECUTABLE_NAME}\" -D NightlySubmit"
  "\"${CTEST_EXECUTABLE_NAME}\" -D NightlyTest -E MPI"
  "\"${CTEST_EXECUTABLE_NAME}\" -D NightlySubmit -A \"${CTEST_BINARY_DIRECTORY}/CMakeCache.txt\" "
)

SET (CTEST_INITIAL_CACHE "

#Trilinos_ENABLE_ALL_PACKAGES:BOOL=ON
#Trilinos_ENABLE_TESTS::BOOL=ON
#Trilinos_ENABLE_EXAMPLES::BOOL=ON
Trilinos_ENABLE_Epetra::BOOL=ON
Epetra_ENABLE_TESTS::BOOL=ON
Epetra_ENABLE_EXAMPLES::BOOL=ON
Trilinos_ENABLE_DEPENCENCY_UNIT_TESTS::BOOL=OFF
Trilinos_ENABLE_FORTRAN:BOOL=OFF
#Teuchos_ENABLE_COMPLEX:BOOL=ON
#Teuchos_ENABLE_EXTENDED:BOOL=ON



TPL_ENABLE_MPI:BOOL=OFF


BUILDNAME:STRING=$ENV{HOSTTYPE}-${TEST_TYPE}-${BUILD_TYPE}

CMAKE_BUILD_TYPE:STRING=${BUILD_TYPE}

CMAKE_C_FLAGS:STRING=-fexceptions

CMAKE_EXE_LINKER_FLAGS:STRING=-lpthread


#MAKECOMMAND:STRING=nmake -i
#CMAKE_MAKE_PROGRAM:FILEPATH=nmake
#CMAKE_GENERATOR:INTERNAL=NMake Makefiles


")


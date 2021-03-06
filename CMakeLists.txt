

CMAKE_MINIMUM_REQUIRED(VERSION 2.7)


#
# A) Basic top-level Trilinos stuff
#

MESSAGE("")
MESSAGE("Configuring Trilinos build directory")
MESSAGE("")

IF ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_BINARY_DIR}")
  MESSAGE(FATAL_ERROR "Error, CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}"
    " == CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}"
    " Trilinos does not support in source builds!" )
ENDIF()

PROJECT(Trilinos NONE)

SET(TRILINOS_HOME_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "")
SET(TRILINOS_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} CACHE INTERNAL "")
# Above, we put these in the cache so we can grep them out

MESSAGE("-- " "TRILINOS_HOME_DIR = ${TRILINOS_HOME_DIR}")
MESSAGE("-- " "TRILINOS_BUILD_DIR = ${TRILINOS_BUILD_DIR}")

IF (NOT ${PROJECT_NAME}_DEPS_HOME_DIR)
  SET(${PROJECT_NAME}_DEPS_HOME_DIR "${TRILINOS_HOME_DIR}")
ENDIF()

SET( CMAKE_MODULE_PATH
   ${TRILINOS_HOME_DIR}
   ${TRILINOS_HOME_DIR}/cmake
   ${TRILINOS_HOME_DIR}/cmake/utils
   ${TRILINOS_HOME_DIR}/cmake/package_arch
   ${TRILINOS_HOME_DIR}/cmake/config_tests
   )

#Pull in Trilinos version information and create the version header file.
INCLUDE(TrilinosVersion)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Trilinos_version.h.in
  Trilinos_version.h)

#Automatically update the version file for sierra too.
IF (NOT WIN32)
  # Only configure on non-windows system because of the dreaded ^M
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/Trilinos_version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/SIERRA/bjam/config_headers/Trilinos_version.h)
ENDIF()

#Since the version header file is now configured the root build
#dir needs to be on the include path
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

INCLUDE(PackageArchGlobalMacros)
INCLUDE(TrilinosGlobalMacros)
INCLUDE(AdvancedSet)
INCLUDE(AdvancedOption)

OPTION(Trilinos_VERBOSE_CONFIGURE
  "Make the Trilinos configure process verbose." OFF)

#
# A.1) Run misc unit tests that don't need anything else
#
# These below tests are a bit strange.  See
# cmake/TestingUnitTests/CMakeLists.txt for details!
#

IF (Trilinos_INVOKE_TESTING_UNIT_TESTS)
  ADD_SUBDIRECTORY(cmake/TestingUnitTests/UnitTests)
  RETURN()
ENDIF()

#
# A.2) Set up other stuff
#

# Find Python executable which is needed for dependency file building
FIND_PACKAGE(PythonInterp)


#
# B) Set up user options and global variables that will be used throughout
#

MESSAGE("")
MESSAGE("Setting up major user options ...")
MESSAGE("")

PACKAGE_ARCH_DEFINE_GLOBAL_OPTIONS()

ADVANCED_SET(Trilinos_DATA_DIR
  "NOTFOUND"
  CACHE PATH
  "Path TrilinosData directory to find more tests and other stuff" )

ADVANCED_OPTION(Trilinos_SHORTCIRCUIT_AFTER_DEPENDENCY_HANDLING
  "Shortcircut after dependency handling is complete"
  OFF )

ADVANCED_OPTION(Trilinos_SKIP_FORTRANCINTERFACE_VERIFY_TEST
  "Skip the Fortran/C++ compatibility test"
  OFF )

SET(TRILINOS_BUILD_SHARED_LIBS "@BUILD_SHARED_LIBS@")


#
# C) Read in Trilinos packages and TPLs and process dependencies
#

PACKAGE_ARCH_READ_PACKAGES_PROCESS_DEPENDENCIES_WRITE_XML()


#
# D) Apply logic to enable Trilinos packages and tests
#

IF (NOT Trilinos_ENABLE_Fortran)
  MESSAGE(
    "\n***"
    "\n*** Warning: Setting Trilinos_ENABLE_ForTrilinos=OFF"
    " because Trilinos_ENABLE_Fortran=OFF!"
    "\n***\n"
    )
  SET(Trilinos_ENABLE_ForTrilinos OFF)
ENDIF()

PACKAGE_ARCH_ADJUST_AND_PRINT_PACKAGE_DEPENDENCIES()


#
# E) Stop if asked
#

IF (Trilinos_SHORTCIRCUIT_AFTER_DEPENDENCY_HANDLING)
  MESSAGE("")
  MESSAGE(SEND_ERROR "Shortcircuiting after dependency tracking ...")
  RETURN()
ENDIF()

# ToDo: rabartl: Remove the above once the unit tests have been refactored to
# just run macros and not the entire system.


#
# F) Add the 'dashboard' target
#

TRILINOS_ADD_DASHBOARD_TARGET()


#
# G) Set up the environment on this computer
#

MESSAGE("")
MESSAGE("Probing the environment ...")
MESSAGE("")

PACKAGE_ARCH_PRE_SETUP_ENV()

INCLUDE(TrilinosProbeEnv)

PACKAGE_ARCH_POST_SETUP_ENV()


#
# H) Go get the information for all enabled TPLS
#

MESSAGE("")
MESSAGE("Getting information for all enabled TPLs ...")
MESSAGE("")

PACKAGE_ARCH_PROCESS_ENABLED_TPLS()

# OpenMP is similar to a TPL in some respects, but requires only compiler
# flags to enable

OPTION(Trilinos_ENABLE_OpenMP
      "Build with OpenMP support." OFF)


#
# I) Set up for testing with CTest and Trilinos test harness
#

MESSAGE("")
MESSAGE("Setting up testing support ...")
MESSAGE("")

INCLUDE(CTest)

TRILINOS_SETUP_TESTING_SUPPORT()


#
# J) Configure individual packages
# 

MESSAGE("")
MESSAGE("Configuring individual enabled Trilinos packages ...")
MESSAGE("")

PACKAGE_ARCH_CONFIGURE_ENABLED_PACKAGES()


#
# K) Setup for packaging and distribution
#

TRILINOS_DEFINE_PACKAGING()


#
# L) Install-related commands
#

PACKAGE_ARCH_WRITE_CONFIG_FILE()


#
# M) Export the library dependencies. This will let client projects
# refer to all TPLs used by Trilinos. (KRL, 26 Nov 2009)
#

MESSAGE("")
MESSAGE("Exporting library dependencies ...")
MESSAGE("")
EXPORT_LIBRARY_DEPENDENCIES( ${Trilinos_BINARY_DIR}/TrilinosLibraryDepends.cmake )

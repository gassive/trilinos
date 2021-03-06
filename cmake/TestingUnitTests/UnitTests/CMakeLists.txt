
# NOTE: The PROJECT_NAME = Trilinos gets set in the driver for this

INCLUDE(MessageWrapper)
INCLUDE(PackageArchCategories)
INCLUDE(PackageAddTest)
INCLUDE(PackageAddAdvancedTest)
INCLUDE(UnitTestHelpers)
INCLUDE(GlobalSet)

#####################################################################
#
# Unit tests for PACKAGE_ADD_XXX(...) CMake commands run as CMake code
#
# This file contains a set of unit tests for the package_arch macros
# PACKAGE_ADD_XXX(...) functions.  These unit tests are written in CMake
# itself.  This is not a very advanced unit testing system and it not that
# easy to work with.  However, it does perform some pretty strong testing and
# is much better than doing nothing.
#
#####################################################################


#
# Set up unit test functions that will be called below to actually run the
# unit tests.
#
# The reason that we use functions is so that we can change varibles just
# inside of the functions that have their own variable scoping.  In that way,
# we can keep variables that are set in one unit test from affecting the
# others.
#


FUNCTION(UNITEST_PACKAGE_ARCH_MISC)

  MESSAGE("\n***")
  MESSAGE("*** Testing miscellaneous PackageArch functionality")
  MESSAGE("***\n")

  MESSAGE("Testing MESSAGE_WRAPPER(...) without capture")
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE FALSE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  MESSAGE_WRAPPER("Some message that should get printed and not intercepted")
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT "")

  MESSAGE("Testing MESSAGE_WRAPPER(...) with capture")
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE TRUE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  MESSAGE_WRAPPER("Some message that should get intercepted")
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT "Some message that should get intercepted")

  MESSAGE("Testing FIND_LIST_ELEMENT(${PROJECT_NAME}_VALID_CATEGORIES BASIC ELEMENT_FOUND)")
  FIND_LIST_ELEMENT(${PROJECT_NAME}_VALID_CATEGORIES BASIC ELEMENT_FOUND)
  UNITTEST_COMPARE_CONST(ELEMENT_FOUND "TRUE")

  MESSAGE("Testing FIND_LIST_ELEMENT(${PROJECT_NAME}_VALID_CATEGORIES BADCAT ELEMENT_FOUND)")
  FIND_LIST_ELEMENT(${PROJECT_NAME}_VALID_CATEGORIES BADCAT ELEMENT_FOUND)
  UNITTEST_COMPARE_CONST(ELEMENT_FOUND "FALSE")

  MESSAGE("Testing PACKAGE_ARCH_GET_INVALID_CATEGORIES( ... BADCAT)")
  PACKAGE_ARCH_GET_INVALID_CATEGORIES(INVALID_CATEGORIES BADCAT)  
  UNITTEST_COMPARE_CONST( INVALID_CATEGORIES "BADCAT" )

  MESSAGE("Testing PACKAGE_ARCH_GET_INVALID_CATEGORIES( ... BADCAT BASIC)")
  PACKAGE_ARCH_GET_INVALID_CATEGORIES(INVALID_CATEGORIES BADCAT BASIC)  
  UNITTEST_COMPARE_CONST( INVALID_CATEGORIES "BADCAT" )

  MESSAGE("Testing PACKAGE_ARCH_GET_INVALID_CATEGORIES( ... BASIC BADCAT)")
  PACKAGE_ARCH_GET_INVALID_CATEGORIES(INVALID_CATEGORIES BASIC BADCAT)  
  UNITTEST_COMPARE_CONST( INVALID_CATEGORIES "BADCAT" )

  MESSAGE("Testing PACKAGE_ARCH_GET_INVALID_CATEGORIES( ... BADCAT1 BADCAT2)")
  PACKAGE_ARCH_GET_INVALID_CATEGORIES(INVALID_CATEGORIES BADCAT1 BADCAT2)  
  UNITTEST_COMPARE_CONST( INVALID_CATEGORIES "BADCAT1;BADCAT2" )

  MESSAGE("Testing PACKAGE_ARCH_GET_INVALID_CATEGORIES( ... BASIC BADCAT1 NIGHTLY BADCAT2 PERFORMANCE)")
  PACKAGE_ARCH_GET_INVALID_CATEGORIES(INVALID_CATEGORIES BASIC BADCAT1 NIGHTLY BADCAT2 PERFORMANCE)  
  UNITTEST_COMPARE_CONST( INVALID_CATEGORIES "BADCAT1;BADCAT2" )

  MESSAGE("Testing PACKAGE_ARCH_ASSERT_VALID_CATEGORIES( ... BADCAT1 BASIC BADCAT2)")
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE TRUE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  PACKAGE_ARCH_ASSERT_VALID_CATEGORIES(BADCAT1 BASIC BADCAT2)  
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE FALSE)
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT
    "SEND_ERROR;Error: The categories 'BADCAT1;BADCAT2' are not; in the list of valid categories '${${PROJECT_NAME}_VALID_CATEGORIES_STR}'!")

  MESSAGE("Testing PACKAGE_ARCH_ASSERT_VALID_CATEGORIES( ... BASIC)")
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE TRUE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  PACKAGE_ARCH_ASSERT_VALID_CATEGORIES(BASIC)  
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE FALSE)
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT "Dummy")

  MESSAGE("Testing PACKAGE_ARCH_ASSERT_VALID_CATEGORIES( ... BASIC NIGHTLY)")
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE TRUE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  PACKAGE_ARCH_ASSERT_VALID_CATEGORIES(BASIC NIGHTLY)  
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE FALSE)
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT "Dummy")

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_TEST_BASIC)

  MESSAGE("\n***")
  MESSAGE("*** Testing basic functionality of PACKAGE_ADD_TEST(...)")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_TEST(...)
  SET(PACKAGE_NAME PackageA)

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN})

  MESSAGE("Test in HOST")
  SET(${PROJECT_NAME}_HOSTNAME MyHost)
  PACKAGE_ADD_TEST( ${EXEN} HOST MyHost )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test not in HOST")
  SET(${PROJECT_NAME}_HOSTNAME TheHost)
  PACKAGE_ADD_TEST( ${EXEN} HOST MyHost )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test in XHOST")
  SET(${PROJECT_NAME}_HOSTNAME MyHost)
  PACKAGE_ADD_TEST( ${EXEN} XHOST MyHost )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test not in XHOST")
  SET(${PROJECT_NAME}_HOSTNAME TheHost)
  PACKAGE_ADD_TEST( ${EXEN} XHOST MyHost )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test in HOSTTYPE")
  SET(CMAKE_HOST_SYSTEM_NAME MyHostType)
  PACKAGE_ADD_TEST( ${EXEN} HOSTTYPE MyHostType )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test not in HOSTTYPE")
  SET(CMAKE_HOST_SYSTEM_NAME TheHostType)
  PACKAGE_ADD_TEST( ${EXEN} HOSTTYPE MyHostType )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test in XHOSTTYPE")
  SET(CMAKE_HOST_SYSTEM_NAME MyHostType)
  PACKAGE_ADD_TEST( ${EXEN} XHOSTTYPE MyHostType )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test not in XHOSTTYPE")
  SET(CMAKE_HOST_SYSTEM_NAME TheHostType)
  PACKAGE_ADD_TEST( ${EXEN} XHOSTTYPE MyHostType )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a single basic test with no arguments")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a single basic test with a single argument")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1"
    )

  MESSAGE("Add a single basic test with two arguments")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add two tests with simple arguments")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1" "arg2 arg3" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_0;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;${PACKEXEN}_1;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg2;arg3"
    )

  MESSAGE("Add a double quoted input argument")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "--arg1=\"bob and cats\"" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;--arg1=\"bob and cats\""
    )

  MESSAGE("Add a double quoted with single quotes input argument")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "--arg1=\"'bob' and 'cats'\"" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;--arg1=\"'bob' and 'cats'\""
    )

  MESSAGE("Add two tests with different postfixes and arguments")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN}
    POSTFIX_AND_ARGS_0 pf_arg1 arg1
    POSTFIX_AND_ARGS_1 pf_arg23 arg2 arg3
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_pf_arg1;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;${PACKEXEN}_pf_arg23;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg2;arg3"
    )

  MESSAGE("Add an executable with no prefix")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NOEXEPREFIX ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${EXEN}.exe;arg1"
    )

  MESSAGE("Add an executable with no suffix")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NOEXESUFFIX ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN};arg1"
    )

  MESSAGE("Add an executable with no prefix and no suffix")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NOEXEPREFIX NOEXESUFFIX ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${EXEN};arg1"
    )

  MESSAGE("Add a test with a different name from the executable")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NAME SomeOtherName ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKAGE_NAME}_SomeOtherName;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1"
    )

  MESSAGE("Add a test with with a postfix appended to the executable name")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NAME_POSTFIX somePostfix ARGS arg1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_somePostfix;${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1"
    )

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_TEST_CATEGORIES)

  MESSAGE("\n***")
  MESSAGE("*** Testing PACKAGE_ADD_TEST( ... CATEGORIES ... )")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_TEST(...)
  SET(PACKAGE_NAME PackageA)

  SET(${PROJECT_NAME}_TEST_CATEGORIES "")

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN})

  MESSAGE("Test no category matching NIGHTLY category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES NIGHTLY)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test no category matching BASIC category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES BASIC)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test NIGHTLY category *not* matching BASIC category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES BASIC)
  PACKAGE_ADD_TEST( ${EXEN} CATEGORIES NIGHTLY )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test BASIC category matching BASIC category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES BASIC)
  PACKAGE_ADD_TEST( ${EXEN} CATEGORIES BASIC )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test no category *not* matching PERFORMANCE category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES PERFORMANCE)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Test PERFORMANCE category matching PERFORMANCE category set by client")
  SET(${PROJECT_NAME}_TEST_CATEGORIES PERFORMANCE)
  PACKAGE_ADD_TEST( ${EXEN} CATEGORIES PERFORMANCE )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Test invlaid BADCAT category not matching anything and resulting in error")
  SET(${PROJECT_NAME}_TEST_CATEGORIES NIGHTLY)
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE TRUE)
  GLOBAL_SET(MESSAGE_WRAPPER_INPUT "Dummy")
  PACKAGE_ADD_TEST( ${EXEN} CATEGORIES BADCAT )
  SET(MESSAGE_WRAPPER_UNIT_TEST_MODE FALSE)
  UNITTEST_COMPARE_CONST(MESSAGE_WRAPPER_INPUT
    "SEND_ERROR;Error: The categories 'BADCAT' are not; in the list of valid categories '${${PROJECT_NAME}_VALID_CATEGORIES_STR}'!")
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "")

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_TEST_COMM)

  MESSAGE("\n***")
  MESSAGE("*** Testing PACKAGE_ADD_TEST( ... COMM ... )")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_TEST(...)
  SET(PACKAGE_NAME PackageB)

  # Doing default serial mode

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN})
  SET(MPI_EXEC_MAX_NUMPROCS 5)
  SET(MPI_EXEC mpiexec)
  SET(MPI_EXEC_PRE_NUMPROCS_FLAGS --pre-num-procs-flags)
  SET(MPI_EXEC_NUMPROCS_FLAG --num-procs)
  SET(MPI_EXEC_POST_NUMPROCS_FLAGS --post-num-procs-flags)

  MESSAGE("Add a test for serial with no COMM input")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a test for MPI with no COMM input")
  SET(TPL_ENABLE_MPI ON)
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  # Doing MPI mode
  SET(TPL_ENABLE_MPI ON)

  MESSAGE("Add a test for MPI with no COMM input but with some args")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add a serial-only in an MPI-only build (adds no test)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} COMM serial )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Add a test for MPI with 'COMM mpi'")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add an MPI test with 2 procs")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi NUM_MPI_PROCS 2 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_2;${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};2;${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add an MPI test with more than the number of allowed processors (will not be added)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi NUM_MPI_PROCS 10 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Add an MPI test with NUM_PROCS 1-10 (will be max num procs)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi NUM_MPI_PROCS 1-10 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add an MPI test with NUM_PROCS 3-10 (will be max num procs)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi NUM_MPI_PROCS 3-10 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  MESSAGE("Add an MPI test with NUM_PROCS ${MPI_EXEC_MAX_NUMPROCS}-10 (will be max num procs)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} ARGS "arg1 arg2" COMM mpi NUM_MPI_PROCS ${MPI_EXEC_MAX_NUMPROCS}-10 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;arg2"
    )

  # Doing serial mode
  SET(TPL_ENABLE_MPI OFF)

  MESSAGE("Add a test for serial mode with 'COMM serial'")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} COMM serial )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a test for serial mode with 'COMM mpi (adds no test)")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} COMM mpi )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    ""
    )

  MESSAGE("Add a test with MPI and NAME_POSTFIX")
  SET(TPL_ENABLE_MPI ON)
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NAME_POSTFIX mypostfix1 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_mypostfix1_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a test with MPI and NAME")
  SET(TPL_ENABLE_MPI ON)
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NAME ${EXEN}_mypostfix2 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_mypostfix2_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe"
    )

  MESSAGE("Add a test with MPI, two arguments, and NAME_POSTFIX")
  SET(TPL_ENABLE_MPI ON)
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_TEST( ${EXEN} NAME_POSTFIX mypostfix3 ARGS "arg1" "arg2" )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_TEST_ADD_TEST_INPUT
    "${PACKEXEN}_mypostfix3_0_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg1;${PACKEXEN}_mypostfix3_1_MPI_${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC};${MPI_EXEC_PRE_NUMPROCS_FLAGS};${MPI_EXEC_NUMPROCS_FLAG};${MPI_EXEC_MAX_NUMPROCS};${MPI_EXEC_POST_NUMPROCS_FLAGS};${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe;arg2"
    )

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_ADVANCED_TEST_BASIC)

  MESSAGE("\n***")
  MESSAGE("*** Testing basic functionality of PACKAGE_ADD_ADVANCED_TEST(...)")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_ADVANCED_TEST(...)
  SET(PACKAGE_NAME PackageA)

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN}.exe)
  SET(CMNDN ls)

  MESSAGE("***\n*** Add a single basic command with no arguments\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_cmnd_1_args_0
    TEST_0 CMND ${CMNDN}
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMNDN}\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_cmnd_1_args_0.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"ls\""
      "NUM_CMNDS 1"
    )

  MESSAGE("***\n*** Add a single package executable with no arguments\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_exec_1_args_0
    TEST_0 EXEC ${EXEN}
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_exec_1_args_0.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\""
      "NUM_CMNDS 1"
    )

  MESSAGE("***\n*** Add a single basic command with two arguments\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_cmnd_1_args_2
    TEST_0 CMND ${CMNDN} ARGS CMakeLists.txt CMakeFiles
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMNDN}\" \"CMakeLists.txt\" \"CMakeFiles\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_cmnd_1_args_2.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"ls\" \"CMakeLists.txt\" \"CMakeFiles\""
      "NUM_CMNDS 1"
      "CMAKE_MODULE_PATH"
      "DriveAdvancedTest"
      "DRIVE_ADVANCED_TEST"
    )

  MESSAGE("***\n*** Add a single package executable with three arguments\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_exec_1_args_3
    TEST_0 EXEC ${EXEN} ARGS arg1 arg2 arg3
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\" \"arg1\" \"arg2\" \"arg3\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_exec_1_args_3.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\" \"arg1\" \"arg2\" \"arg3\""
      "NUM_CMNDS 1"
    )

  MESSAGE("***\n*** Add two basic commands with 1 and two arguments\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_cmnd_2_args_1_2
    TEST_0 CMND echo ARGS "Cats and Dogs"
    TEST_1 CMND ls ARGS Cats
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    2
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"echo\" \"Cats and Dogs\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_1
    "\"ls\" \"Cats\""
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_cmnd_2_args_1_2.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"echo\" \"Cats and Dogs\""
      "TEST_1_CMND \"ls\" \"Cats\""
      "NUM_CMNDS 2"
    )

  MESSAGE("***\n*** Add a single basic command matching HOST\n***")
  SET(${PROJECT_NAME}_HOSTNAME MyHost)
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_host
    TEST_0 CMND ${CMNDN}
    HOST MyHost
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMNDN}\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_PackageAddAdvancedTest_basic_host.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"ls\""
      "NUM_CMNDS 1"
    )

  MESSAGE("***\n*** Add a single basic command not matching HOST\n***")
  SET(${PROJECT_NAME}_HOSTNAME MyHost)
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_basic_host
    TEST_0 CMND ${CMNDN}
    HOST NotMyHost
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    ""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    ""
    )

  # ToDo: Add 6 more tests testing XHOST, HOSTTYPE, and XHOSTTYPE

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_ADVANCED_TEST_CATEGORIES)

  MESSAGE("\n***")
  MESSAGE("*** Testing PACKAGE_ADD_ADVANCED_TEST( ... CATEGORIES ... )")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_TEST(...)
  SET(PACKAGE_NAME PackageA)

  SET(${PROJECT_NAME}_TEST_CATEGORIES "")

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN}.exe)

  MESSAGE("Test no category matching NIGHTLY category set by client")
  SET(TEST_NAME PackageAddAdvancedTestCategory_Empty_Nighly)
  SET(${PROJECT_NAME}_TEST_CATEGORIES NIGHTLY)
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( ${TEST_NAME} TEST_0 EXEC ${EXEN} )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${TEST_NAME}.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\""
      "NUM_CMNDS 1"
    )

  MESSAGE("Test no category *not* matching PERFORMANCE category set by client")
  SET(TEST_NAME PackageAddAdvancedTestCategory_Empty_Performance)
  SET(${PROJECT_NAME}_TEST_CATEGORIES PERFORMANCE)
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( ${TEST_NAME} TEST_0 EXEC ${EXEN} )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    ""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    ""
    )

  MESSAGE("Test PERFORMANCE category matching PERFORMANCE category set by client")
  SET(TEST_NAME PackageAddAdvancedTestCategory_Performance_Performance)
  SET(${PROJECT_NAME}_TEST_CATEGORIES PERFORMANCE)
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( ${TEST_NAME} TEST_0 EXEC ${EXEN} CATEGORIES PERFORMANCE )
  UNITTEST_FILE_REGEX(
    "${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_${TEST_NAME}.cmake"
    REGEX_STRINGS
      "TEST_0_CMND \"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}\""
      "NUM_CMNDS 1"
    )

  # NOTE: The above tests ensure that the CATEGORIES argument is accepted and
  # processed correctly.  The unit tests in
  # UNIT_TEST_PACKAGE_ADD_TEST_CATEGORIES() test the behavior of the logic for
  # selecting tests based on CATEGORIES.

ENDFUNCTION()


FUNCTION(UNITEST_PACKAGE_ADD_ADVANCED_TEST_COMM)

  MESSAGE("\n***")
  MESSAGE("*** Testing PACKAGE_ADD_ADVANCED_TEST( ... COMM ... )")
  MESSAGE("***\n")

  # Needed by PACKAGE_ADD_ADVANCED_TEST(...)
  SET(PACKAGE_NAME PackageA)

  # Used locally
  SET(EXEN SomeExec)
  SET(PACKEXEN ${PACKAGE_NAME}_${EXEN})
  SET(MPI_EXEC_MAX_NUMPROCS 5)
  SET(MPI_EXEC mpiexec)
  SET(MPI_EXEC_PRE_NUMPROCS_FLAGS --pre-num-procs-flags)
  SET(MPI_EXEC_NUMPROCS_FLAG --num-procs)
  SET(MPI_EXEC_POST_NUMPROCS_FLAGS --post-num-procs-flags)
  SET(CMNDN ls)

  SET(PACKAGE_ADD_ADVANCED_TEST_SKIP_SCRIPT TRUE)

  # Default serial mode

  MESSAGE("***\n*** Add a test with no COMM argument\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( DummyTest
    TEST_0 CMND ${CMNDN}
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMNDN}\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )

  MESSAGE("***\n*** Add a serial test with 'COMM serial'\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( DummyTest
    TEST_0 CMND ${CMNDN}
    COMM serial
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${CMNDN}\""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )

  # Doing MPI mode
  SET(TPL_ENABLE_MPI ON)

  MESSAGE("***\n*** Add serial-only test with 'COMM mpi' (will not add the test)\n***")
  PACKAGE_ADD_ADVANCED_TEST_UNITTEST_RESET()
  PACKAGE_ADD_ADVANCED_TEST( DummyTest
    TEST_0 CMND ${CMNDN}
    COMM serial
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    ""
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    ""
    )

  MESSAGE("***\n*** Add an advanced test for MPI with no COMM input but with two args\n***")
  GLOBAL_SET(PACKAGE_ADD_TEST_ADD_TEST_INPUT)
  PACKAGE_ADD_ADVANCED_TEST( PackageAddAdvancedTest_mpi_exec_1_args_2
    TEST_0 EXEC ${EXEN} ARGS arg1 arg2 )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_NUM_CMNDS
    1
    )
  UNITTEST_COMPARE_CONST(
    PACKAGE_ADD_ADVANCED_TEST_CMND_ARRAY_0
    "\"${MPI_EXEC}\" \"${MPI_EXEC_PRE_NUMPROCS_FLAGS}\" \"${MPI_EXEC_NUMPROCS_FLAG}\" \"${MPI_EXEC_MAX_NUMPROCS}\" \"${MPI_EXEC_POST_NUMPROCS_FLAGS}\" \"${CMAKE_CURRENT_BINARY_DIR}/${PACKEXEN}.exe\" \"arg1\" \"arg2\""
    )

ENDFUNCTION()


#
# Execute the unit tests
#


# Set up some global environment stuff
SET(${PROJECT_NAME}_HOSTNAME testhost.nowhere.com)

# Assume that all unit tests will pass by default
GLOBAL_SET(UNITTEST_OVERALL_PASS TRUE)
GLOBAL_SET(UNITTEST_OVERALL_NUMPASSED 0)
GLOBAL_SET(UNITTEST_OVERALL_NUMRUN 0)


# Set up the PACKAGE_ADD_TEST(...) function to only capture the output and not
# actually add the test.
SET( PACKAGE_ADD_TEST_ADD_TEST_CAPTURE TRUE )
SET( PACKAGE_ADD_TEST_ADD_TEST_SKIP TRUE )

UNITEST_PACKAGE_ARCH_MISC()

# Set the default test categories
SET(${PROJECT_NAME}_TEST_CATEGORIES NIGHTLY)

MESSAGE("\n***")
MESSAGE("*** Testing PACAKGE_ADD_TEST(...)")
MESSAGE("***\n")

UNITEST_PACKAGE_ADD_TEST_BASIC()
UNITEST_PACKAGE_ADD_TEST_CATEGORIES()
UNITEST_PACKAGE_ADD_TEST_COMM()


MESSAGE("\n***")
MESSAGE("*** Testing PACAKGE_ADD_ADVANCED_TEST(...)")
MESSAGE("***\n")

UNITEST_PACKAGE_ADD_ADVANCED_TEST_BASIC()
UNITEST_PACKAGE_ADD_ADVANCED_TEST_CATEGORIES()
UNITEST_PACKAGE_ADD_ADVANCED_TEST_COMM()

MESSAGE("\n***")
MESSAGE("*** Determine final result of all unit tests")
MESSAGE("***\n")

# Pass in the number of expected tests that must pass!
UNITTEST_FINAL_RESULT(100)

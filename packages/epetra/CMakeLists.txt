
INCLUDE(PackageMacros)

PACKAGE(Epetra)

SET(${PACKAGE_NAME}_VERSION "3.7d")

ADD_OPTION_AND_DEFINE(${PACKAGE_NAME}_ENABLE_ABC
  HAVE_EPETRA_ARRAY_BOUNDS_CHECK
  "Enable array bounds checking on Epetra objects."
  ${Trilinos_ENABLE_DEBUG} )

ADD_OPTION_AND_DEFINE(${PACKAGE_NAME}_ENABLE_FORMAT_IO
  HAVE_FORMAT_IO
  "Enable format IO capability."
  OFF )

ADD_OPTION_AND_DEFINE(${PACKAGE_NAME}_ENABLE_WARNING_MESSAGES
  HAVE_WARNING_MESSAGES
  "Enable building with warning messages."
  OFF )

ADD_OPTION_AND_DEFINE(${PACKAGE_NAME}_ENABLE_FATAL_MESSAGES
  HAVE_FATAL_MESSAGES
  "Enable printing of fatal error messages."
  OFF )

ADD_OPTION_AND_DEFINE(${PACKAGE_NAME}_ENABLE_THREADS
  HAVE_THREADS
  "Enable threading support."
  OFF )

ADD_SUBDIRECTORY(src)

PACKAGE_ADD_TEST_DIRECTORIES(test)

PACKAGE_ADD_EXAMPLE_DIRECTORIES(example)

#
# Exclude files for source package.
#

PACKAGE_ARCH_EXCLUDE_AUTOTOOLS_FILES()

PACKAGE_ARCH_EXCLUDE_FILES(
  doc
  example/C_wrappers
  example/Fortran
  example/ImportExport
  example/InverseIteration
  example/MapColoring
  example/ReducedLinearProblem
  example/petra_howle
  example/petra_nonlinear
  example/petra_transpose
  src/Epetra_FastCrsMatrix.cpp
  src/Epetra_FastCrsMatrix.h
  src/Epetra_InvOperator.cpp
  src/Epetra_LinearProblemRedistor.cpp
  src/Epetra_LinearProblemRedistor.h
  src/Epetra_MpiSmpComm.*
  src/stamp-h.in
  src/xxemacs
  test/BasicPerfTest/runSummary
  test/Comm/simple_mpi.cpp
  test/Comm/threaded_Makefile
  test/Comm/threaded_main.cpp
  test/EpetraBenchmarkTest
  test/LinearProblemRedistor
  test/Makefile.template
  test/Map/c_main.c
  test/MultiVector/Makefile.purify
  test/OSKI
  test/VbrMatrix/Suppressions.in
  test/Vector/Makefile.purify
  test/testAll.*
  )

PACKAGE_POSTPROCESS()

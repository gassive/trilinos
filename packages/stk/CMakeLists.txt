
INCLUDE(PackageMacros)
INCLUDE(AddOptionAndDefine)

#
# A) Define the package
#

# ENABLE_SHADOWING_WARNINGS  not possible, because of TBB
PACKAGE( STK )

#
# B) Set up package-specific options
#

ADD_OPTION_AND_DEFINE(
  ${PACKAGE_NAME}_Trilinos
  HAVE_STK_Trilinos
  "Building as a Trilinos package"
  ON
  )


#
# C) Add the libraries, tests, and examples
#

ADD_SUBDIRECTORY(stk_util)

ADD_SUBDIRECTORY(stk_mesh)

IF( STK_ENABLE_Trios )
  ADD_SUBDIRECTORY(stk_io)
ENDIF()

IF( STK_ENABLE_FEI AND STK_ENABLE_AztecOO )
  ADD_SUBDIRECTORY(stk_linsys)
ENDIF()

ADD_SUBDIRECTORY(stk_transfer)

IF( STK_ENABLE_ThreadPool )
  ADD_SUBDIRECTORY(stk_search)
ENDIF()

#
# D) Do standard postprocessing
#

PACKAGE_POSTPROCESS()


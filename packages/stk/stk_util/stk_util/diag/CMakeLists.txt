
INCLUDE(PackageLibraryMacros)

#
# A) Package-specific configuration options
#

# PACKAGE_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)

#
# B) Define the header and source files (and directories)
#

#
# src
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../)

#
# Core headers
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

APPEND_SET(HEADERS
PrintTimer.hpp
Timer.hpp
TimerMetricTraits.hpp
WriterExt.hpp
Writer_fwd.hpp
Writer.hpp
WriterManip.hpp
WriterOStream.hpp
  )

APPEND_SET(SOURCES
PrintTimer.cpp
Timer.cpp
TimerMetricTraits.cpp
Writer.cpp
WriterExt.cpp
WriterManip.cpp
  )

IF (TPL_ENABLE_MPI)
ENDIF()

#
# C) Define the targets for package's library(s)
#

PACKAGE_ADD_LIBRARY(
  stk_util_diag
  NOINSTALLHEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )

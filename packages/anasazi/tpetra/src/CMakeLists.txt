
INCLUDE(PackageLibraryMacros)

#
# A) Package-specific configuration options
#

#
# B) Define the header and source files (and directories)
#

#
# src
#

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#
# Core Files
#

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

APPEND_SET(HEADERS
  AnasaziTpetraAdapter.hpp 
  )

APPEND_SET(SOURCES
  AnasaziTpetraAdapter.cpp
  )

#
# C) Define the targets for package's library(s)
#

PACKAGE_ADD_LIBRARY(
  anasazitpetra
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  DEPLIBS anasazi tpetra kokkos
  )

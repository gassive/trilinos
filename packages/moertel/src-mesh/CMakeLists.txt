
INCLUDE(PackageLibraryMacros)

SET(HEADERS "")
SET(SOURCES "")

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# Base library
APPEND_SET(HEADERS
  mrtr_mesh.H
  )

APPEND_SET(SOURCES
  mrtr_mesh.cpp
  )

PACKAGE_ADD_LIBRARY(
		moertelmesh
		HEADERS ${HEADERS}
		SOURCES ${SOURCES}
		)

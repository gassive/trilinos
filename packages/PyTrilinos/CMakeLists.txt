# @HEADER
# ************************************************************************
#
#                PyTrilinos: Python Interface to Trilinos
#                   Copyright (2009) Sandia Corporation
#
# Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
# license for use of this work by or on behalf of the U.S. Government.
#
# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# Questions? Contact Bill Spotz (wfspotz@sandia.gov)
#
# ************************************************************************
# @HEADER

# Include the package macros
INCLUDE(PackageMacros)

# Set the package name
PACKAGE(PyTrilinos DISABLE_STRONG_WARNINGS)

IF(NOT BUILD_SHARED_LIBS)
  MESSAGE(FATAL_ERROR "PyTrilinos can only be built with shared libraries. Building of shared libraries is currently set to OFF. To enable shared libraries please set the cache variable \"BUILD_SHARED_LIBS\" to ON")
ENDIF()

# Set the package version number
SET(${PACKAGE_NAME}_VERSION 4.4)

# Define the list of PyTrilinos packages and modules.  Packages are
# the top-level names only, such as Epetra, AztecOO and NOX.  The list
# of modules is similar to the packages, except it contains the names
# of python modules that correspond to nested namespaces, such as
# NOX.__init__ or NOX.Epetra.Interface.
SET(${PACKAGE_NAME}_PACKAGES "")
SET(${PACKAGE_NAME}_MODULES  "")

IF(PyTrilinos_ENABLE_Teuchos)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Teuchos)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Teuchos)
ENDIF(PyTrilinos_ENABLE_Teuchos)

IF(PyTrilinos_ENABLE_Epetra)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Epetra)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Epetra)
ENDIF(PyTrilinos_ENABLE_Epetra)

IF(PyTrilinos_ENABLE_Triutils)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES TriUtils)
  APPEND_SET(${PACKAGE_NAME}_MODULES  TriUtils)
ENDIF(PyTrilinos_ENABLE_Triutils)

IF(PyTrilinos_ENABLE_EpetraExt)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES EpetraExt)
  APPEND_SET(${PACKAGE_NAME}_MODULES  EpetraExt)
ENDIF(PyTrilinos_ENABLE_EpetraExt)

IF(PyTrilinos_ENABLE_Pliris)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Pliris)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Pliris)
ENDIF(PyTrilinos_ENABLE_Pliris)

IF(PyTrilinos_ENABLE_AztecOO)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES AztecOO)
  APPEND_SET(${PACKAGE_NAME}_MODULES  AztecOO)
ENDIF(PyTrilinos_ENABLE_AztecOO)

IF(PyTrilinos_ENABLE_Galeri)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Galeri)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Galeri)
ENDIF(PyTrilinos_ENABLE_Galeri)

IF(PyTrilinos_ENABLE_Amesos)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Amesos)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Amesos)
ENDIF(PyTrilinos_ENABLE_Amesos)

IF(PyTrilinos_ENABLE_Ifpack)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES IFPACK)
  APPEND_SET(${PACKAGE_NAME}_MODULES  IFPACK)
ENDIF(PyTrilinos_ENABLE_Ifpack)

IF(PyTrilinos_ENABLE_Komplex)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Komplex)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Komplex)
ENDIF(PyTrilinos_ENABLE_Komplex)

IF(PyTrilinos_ENABLE_Anasazi)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES Anasazi)
  APPEND_SET(${PACKAGE_NAME}_MODULES  Anasazi)
ENDIF(PyTrilinos_ENABLE_Anasazi)

IF(PyTrilinos_ENABLE_ML)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES ML)
  APPEND_SET(${PACKAGE_NAME}_MODULES  ML)
ENDIF(PyTrilinos_ENABLE_ML)

IF(PyTrilinos_ENABLE_NOX)
  APPEND_SET(${PACKAGE_NAME}_PACKAGES NOX)
  APPEND_SET(${PACKAGE_NAME}_MODULES  NOX.__init__  )
  APPEND_SET(${PACKAGE_NAME}_MODULES  NOX.Abstract  )
  APPEND_SET(${PACKAGE_NAME}_MODULES  NOX.StatusTest)
  APPEND_SET(${PACKAGE_NAME}_MODULES  NOX.Solver    )
  IF(NOX_ENABLE_Epetra)
    APPEND_SET(${PACKAGE_NAME}_MODULES NOX.Epetra.__init__ )
    APPEND_SET(${PACKAGE_NAME}_MODULES NOX.Epetra.Interface)
  ENDIF(NOX_ENABLE_Epetra)
ENDIF(PyTrilinos_ENABLE_NOX)

#IF(NOX_ENABLE_LOCA)
#  APPEND_SET(${PACKAGE_NAME}_PACKAGES LOCA)
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.__init__         )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Abstract         )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Extended         )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.BorderedSystem   )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Continuation     )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.MultiContinuation)
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Hopf             )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.TimeDependent    )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Pitchfork        )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Homotopy         )
#  APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.TurningPoint     )
#  #APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Bifurcation      )
#  #APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Chan             )
#  IF(NOX_ENABLE_Epetra)
#    APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Epetra.__init__ )
#    APPEND_SET(${PACKAGE_NAME}_MODULES  LOCA.Epetra.Interface)
#  ENDIF(NOX_ENABLE_Epetra)
#ENDIF(NOX_ENABLE_LOCA)

# Python interpreter and header files are required
FIND_PACKAGE(PythonInterp 2.2 REQUIRED)
FIND_PACKAGE(PythonInclude REQUIRED)

# Python module numpy is required
FIND_PACKAGE(NumPy 1.0 REQUIRED)

# Simple wrapper interface generator (SWIG) is required
FIND_PACKAGE(SWIG 1.3.38 REQUIRED)
# ${SWIG_USE_FILE} is hard-coded to be the installed UseSWIG.cmake
# file.  We want to include the local, modified version.
INCLUDE(UseSWIG)

# Print the enabled PyTrilinos packages
MESSAGE(STATUS "Enabled ${PACKAGE_NAME} modules:")
MESSAGE(STATUS "    ${${PACKAGE_NAME}_PACKAGES}")

# Determine the PyTrilinos default installation directory prefix.  If
# CMAKE_INSTALL_PREFIX is set by the user, then use it.  Else the
# prefix is set to the value of python's sys.prefix.
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.prefix"
  OUTPUT_VARIABLE PYTHON_PREFIX
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(PyTrilinos_DEFAULT_INSTALL_PREFIX ${PYTHON_PREFIX})
ELSE(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(PyTrilinos_DEFAULT_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

# Set the PyTrilinos install prefix
SET(PyTrilinos_INSTALL_PREFIX ${PyTrilinos_DEFAULT_INSTALL_PREFIX}
  CACHE PATH "The path prefix for where PyTrilinos will be installed, e.g. /usr/local")

# Add the src directory, which builds the pytrilinos library,
# generates the wrapper code, builds the PyTrilinos python package and
# compiles the PyTrilinos extension modules
ADD_SUBDIRECTORY(src)

# Optionally add the secondary directory for testing the Teuchos::RCP
# wrappers
IF(PyTrilinos_ENABLE_RCPTESTS)
  ADD_SUBDIRECTORY(teuchos_rcp)
ENDIF(PyTrilinos_ENABLE_RCPTESTS)

# Add the test scripts
PACKAGE_ADD_TEST_DIRECTORIES(test)

# Add the example scripts
PACKAGE_ADD_EXAMPLE_DIRECTORIES(example)

#
# Exclude files for source package.
#

PACKAGE_ARCH_EXCLUDE_AUTOTOOLS_FILES()

PACKAGE_ARCH_EXCLUDE_FILES(
  Notes.txt
  aclocal.m4
  bootstrap
  config
  lib
  macdist
  shared
  src/PyTrilinos_config.h.in
  src/depend
  src/setup.py
  src-boost
  )

# Execute the package postprocessing
PACKAGE_POSTPROCESS()

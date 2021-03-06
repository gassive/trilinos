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

INCLUDE(PackageLibraryMacros)

#
# Package-specific configuration options
IF(Trilinos_ENABLE_Epetra)
  SET(HAVE_EPETRA ON)
ENDIF(Trilinos_ENABLE_Epetra)

IF(Trilinos_ENABLE_Teuchos)
  SET(HAVE_TEUCHOS ON)
ENDIF(Trilinos_ENABLE_Teuchos)

IF(NOX_ENABLE_Epetra)
  SET(HAVE_NOX_EPETRA ON)
ENDIF(NOX_ENABLE_Epetra)

IF(NOX_ENABLE_EpetraExt)
  SET(HAVE_NOX_EPETRAEXT ON)
ENDIF(NOX_ENABLE_EpetraExt)

IF(TPL_ENABLE_MPI)
  SET(HAVE_MPI ON)
ENDIF(TPL_ENABLE_MPI)

PACKAGE_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)

#
# On Mac OS X Gnu compilers, add dynamic lookup for undefined symbols
# to the pytrilinos library and PyTrilinos extension modules
SET(EXTRA_LINK_ARGS "")
IF(APPLE)
  IF(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    APPEND_SET(EXTRA_LINK_ARGS "-undefined dynamic_lookup")
  ENDIF()
ENDIF(APPLE)

#
# Add the current binary and source directories, the python include
# path, and the NumPy include path to the include directories
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
INCLUDE_DIRECTORIES(${NumPy_INCLUDE_DIR})

#
# Copy the pyfragments.swg file to the build directory
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pyfragments.swg.in
  ${CMAKE_CURRENT_BINARY_DIR}/pyfragments.swg)

#
# Initialize the headers and sources
SET(HEADERS "")
SET(SOURCES "")
SET(HEADERS ${HEADERS}
  ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}_config.h
  )

#
# Core PyTrilinos headers and sources
APPEND_SET(HEADERS
  NumPyImporter.h
  PythonException.h
  PyTrilinos_Util.h
  FILEstream.h
  numpy_include.h
  )

APPEND_SET(SOURCES
  NumPyImporter.cpp
  PythonException.cpp
  PyTrilinos_Util.cpp
  FILEstream.cpp
  )

#
# PyTrilinos + Teuchos headers and sources
IF(PyTrilinos_ENABLE_Teuchos)

  APPEND_SET(HEADERS
    Teuchos_PythonParameter.h
    )

  APPEND_SET(SOURCES
    Teuchos_PythonParameter.cpp
    )

ENDIF(PyTrilinos_ENABLE_Teuchos)

#
# PyTrilinos + Epetra headers and sources
IF(PyTrilinos_ENABLE_Epetra)

  APPEND_SET(HEADERS
    Epetra_NumPyFEVector.h
    Epetra_NumPyIntSerialDenseMatrix.h
    Epetra_NumPyIntSerialDenseVector.h
    Epetra_NumPyIntVector.h
    Epetra_NumPyMultiVector.h
    Epetra_NumPySerialDenseMatrix.h
    Epetra_NumPySerialDenseVector.h
    Epetra_NumPyVector.h
    Epetra_PyUtil.h
    )

  APPEND_SET(SOURCES
    Epetra_NumPyFEVector.cpp
    Epetra_NumPyIntSerialDenseMatrix.cpp
    Epetra_NumPyIntSerialDenseVector.cpp
    Epetra_NumPyIntVector.cpp
    Epetra_NumPyMultiVector.cpp
    Epetra_NumPySerialDenseMatrix.cpp
    Epetra_NumPySerialDenseVector.cpp
    Epetra_NumPyVector.cpp
    Epetra_PyUtil.cpp
    )

ENDIF(PyTrilinos_ENABLE_Epetra)

#
# PyTrilinos + EpetraExt headers and sources
IF(PyTrilinos_ENABLE_EpetraExt)

  APPEND_SET(HEADERS
    EpetraExt_PyUtil.h
    )

  APPEND_SET(SOURCES
    EpetraExt_PyUtil.cpp
    )

ENDIF(PyTrilinos_ENABLE_EpetraExt)

#
# PyTrilinos + ML headers and sources
IF(PyTrilinos_ENABLE_ML)

  APPEND_SET(HEADERS
    MLAPI_PyMatrix.h
    )

ENDIF(PyTrilinos_ENABLE_ML)

#
# Define the target for the swig runtime header
SET(SWIG_RUNTIME swigpyrun.h)
ADD_CUSTOM_COMMAND(
  OUTPUT ${SWIG_RUNTIME}
  COMMAND ${SWIG_EXECUTABLE} -python -external-runtime
  )
APPEND_SET(HEADERS ${CMAKE_CURRENT_BINARY_DIR}/${SWIG_RUNTIME})

#
# Define the targets for the PyTrilinos library
PACKAGE_ADD_LIBRARY(pytrilinos
  HEADERS ${HEADERS}
  SOURCES ${SOURCES}
  )

SET_TARGET_PROPERTIES(pytrilinos
  PROPERTIES LINK_FLAGS "${EXTRA_LINK_ARGS}"
  )

#
# Define the PyTrilinos swig setup
SET(CMAKE_SWIG_FLAGS "")
GET_FILENAME_COMPONENT(PYTRILINOS_DOXDIR
  ${CMAKE_CURRENT_SOURCE_DIR}/../doc/Doxygen ABSOLUTE
  )
APPEND_SET(CMAKE_SWIG_FLAGS -I${PYTRILINOS_DOXDIR})

#
# Determine the install directory
EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print sys.version[:3]"
  OUTPUT_VARIABLE PYTHON_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
SET(PyTrilinos_INSTALL_DIR
  ${PyTrilinos_INSTALL_PREFIX}/lib/python${PYTHON_VERSION}/site-packages/PyTrilinos
  CACHE PATH "The path where PyTrilinos will be installed"
  )
MESSAGE(STATUS "PyTrilinos installation path: ${PyTrilinos_INSTALL_DIR}")

#
# Add the PyTrilinos subdirectory, which is the top-level package
# directory.  This needs to be done before the following loop over the
# PyTrilinos modules, because of the call to GET_DIRECTORY_PROPERTY().
ADD_SUBDIRECTORY(PyTrilinos)

#
# Initialize the list of additional clean files
SET(ADDITIONAL_CLEAN_FILES "")

#
# Loop over the PyTrilinos-enabled Trilinos modules and define the
# swig-generated PyTrilinos extension modules
FOREACH(MODULE ${PyTrilinos_MODULES})
  # Alternate versions of the module name
  STRING(TOUPPER ${MODULE} MODULE_UPPER)
  STRING(REPLACE "." "_" MODULE_NODOT ${MODULE})
  SET_SOURCE_FILES_PROPERTIES(${MODULE}.i PROPERTIES
    CPLUSPLUS ON
    )
  # Determine the swig module output directory, name and unique target name
  SWIG_MODULE_GET_OUTDIR_AND_MODULE(${MODULE}.i MODULE_OUTDIR MODULE_NAME)
  SET(PyTrilinos_TARGET_NAME ${MODULE_NODOT})
  SWIG_ADD_MODULE(${PyTrilinos_TARGET_NAME}
    python
    "${MODULE}.i"
    "${MODULE_OUTDIR}"
    "${MODULE_NAME}")
  SWIG_LINK_LIBRARIES(${PyTrilinos_TARGET_NAME} pytrilinos)
  # Add the header file to the list of additional "make clean" files
  LIST(APPEND ADDITIONAL_CLEAN_FILES "${MODULE}PYTHON_wrap.h")
  #
  # Choosing the Trilinos libraries to link against, the python module
  # to byte-compile and the installation rules requires a little bit
  # of logic.
  #
  # PyTrilinos.NOX.Epetra
  IF("${MODULE_OUTDIR}" MATCHES "PyTrilinos/NOX/Epetra")
    SET(TRILINOS_LIBS ${NOXEPETRA_LIBRARIES})
    ADD_CUSTOM_COMMAND(TARGET ${PyTrilinos_TARGET_NAME} POST_BUILD
      COMMAND ${PYTHON_EXECUTABLE} -c "import py_compile; py_compile.compile('PyTrilinos/NOX/Epetra/${MODULE_NAME}.py')"
      COMMENT "Byte compiling NOX/Epetra/${MODULE_NAME}.py"
      VERBATIM)
    INSTALL(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/NOX/Epetra/${MODULE_NAME}.py
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/NOX/Epetra/${MODULE_NAME}.pyc
      DESTINATION ${PyTrilinos_INSTALL_DIR}/NOX/Epetra)
    INSTALL(TARGETS ${PyTrilinos_TARGET_NAME}
      DESTINATION ${PyTrilinos_INSTALL_DIR}/NOX/Epetra)
    LIST(APPEND ADDITIONAL_CLEAN_FILES "PyTrilinos/NOX/Epetra/${MODULE_NAME}.pyc")
  #
  # PyTrilinos.NOX
  ELSEIF("${MODULE_OUTDIR}" MATCHES "PyTrilinos/NOX")
    SET(TRILINOS_LIBS ${NOX_LIBRARIES})
    ADD_CUSTOM_COMMAND(TARGET ${PyTrilinos_TARGET_NAME} POST_BUILD
      COMMAND ${PYTHON_EXECUTABLE} -c "import py_compile; py_compile.compile('PyTrilinos/NOX/${MODULE_NAME}.py')"
      COMMENT "Byte compiling NOX/${MODULE_NAME}.py"
      VERBATIM)
    INSTALL(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/NOX/${MODULE_NAME}.py
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/NOX/${MODULE_NAME}.pyc
      DESTINATION ${PyTrilinos_INSTALL_DIR}/NOX)
    INSTALL(TARGETS ${PyTrilinos_TARGET_NAME}
      DESTINATION ${PyTrilinos_INSTALL_DIR}/NOX)
    LIST(APPEND ADDITIONAL_CLEAN_FILES "PyTrilinos/NOX/${MODULE_NAME}.pyc")
  #
  # PyTrilinos.LOCA
  ELSEIF("${MODULE_OUTDIR}" MATCHES "PyTrilinos/LOCA")
    SET(TRILINOS_LIBS ${LOCA_LIBRARIES})
    ADD_CUSTOM_COMMAND(TARGET ${PyTrilinos_TARGET_NAME} POST_BUILD
      COMMAND ${PYTHON_EXECUTABLE} -c "import py_compile; py_compile.compile('PyTrilinos/LOCA/${MODULE_NAME}.py')"
      COMMENT "Byte compiling LOCA/${MODULE_NAME}.py"
      VERBATIM)
    INSTALL(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/LOCA/${MODULE_NAME}.py
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/LOCA/${MODULE_NAME}.pyc
      DESTINATION ${PyTrilinos_INSTALL_DIR}/LOCA)
    INSTALL(TARGETS ${PyTrilinos_TARGET_NAME}
      DESTINATION ${PyTrilinos_INSTALL_DIR}/LOCA)
    LIST(APPEND ADDITIONAL_CLEAN_FILES "PyTrilinos/LOCA/${MODULE_NAME}.pyc")
  #
  # PyTrilinos
  ELSE("${MODULE_OUTDIR}")
    SET(TRILINOS_LIBS ${${MODULE_UPPER}_LIBRARIES})
    ADD_CUSTOM_COMMAND(TARGET ${PyTrilinos_TARGET_NAME} POST_BUILD
      COMMAND ${PYTHON_EXECUTABLE} -c "import py_compile; py_compile.compile('PyTrilinos/${MODULE}.py')"
      COMMENT "Byte compiling ${MODULE}.py"
      VERBATIM)
    INSTALL(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/${MODULE_NAME}.py
      ${CMAKE_CURRENT_BINARY_DIR}/PyTrilinos/${MODULE_NAME}.pyc
      DESTINATION ${PyTrilinos_INSTALL_DIR})
    INSTALL(TARGETS ${PyTrilinos_TARGET_NAME}
      DESTINATION ${PyTrilinos_INSTALL_DIR})
    LIST(APPEND ADDITIONAL_CLEAN_FILES "PyTrilinos/${MODULE}.pyc")
  #
  ENDIF("${MODULE_OUTDIR}" MATCHES "PyTrilinos/NOX/Epetra")
  SWIG_LINK_LIBRARIES(${PyTrilinos_TARGET_NAME} ${TRILINOS_LIBS})
  SET_TARGET_PROPERTIES(${PyTrilinos_TARGET_NAME} PROPERTIES
    LINK_FLAGS "${EXTRA_LINK_ARGS}"
    )
ENDFOREACH(MODULE ${PyTrilinos_MODULES})

#
# Add the additional "make clean" files
GET_DIRECTORY_PROPERTY(clean_files ADDITIONAL_MAKE_CLEAN_FILES)
LIST(APPEND            clean_files ${ADDITIONAL_CLEAN_FILES})
LIST(REMOVE_DUPLICATES clean_files)
LIST(REMOVE_ITEM       clean_files "")
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${clean_files}")

# Create the PyTrilinos_PACKAGES string
SET(${PACKAGE_NAME}_LIST "")
FOREACH(PACKAGE ${${PACKAGE_NAME}_PACKAGES})
  SET(${PACKAGE_NAME}_LIST "${${PACKAGE_NAME}_LIST}    ${PACKAGE}\n")
ENDFOREACH(PACKAGE ${${PACKAGE_NAME}_PACKAGES})

# Create the PyTrilinos_ALL string
SET(${PACKAGE_NAME}_ALL "__all__ = [")
FOREACH(PACKAGE ${${PACKAGE_NAME}_PACKAGES})
  SET(${PACKAGE_NAME}_ALL "${${PACKAGE_NAME}_ALL}'${PACKAGE}',\n          ")
ENDFOREACH(PACKAGE ${${PACKAGE_NAME}_PACKAGES})
SET(${PACKAGE_NAME}_ALL "${${PACKAGE_NAME}_ALL}]")

# Define a trigger target
ADD_CUSTOM_TARGET(Pure_Python_Modules ALL)

# Define the list of local modules
SET(LOCAL_MODULES __init__ PropertyBase typed_tuple typed_list typed_dict)

# Initialize the additional "make clean" files
SET(ADDITIONAL_CLEAN_FILES "")

# Create each local module, its byte-compiled file and its
# installation rules
FOREACH(MODULE ${LOCAL_MODULES})

  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}.src
    ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.py)
  ADD_CUSTOM_COMMAND(TARGET Pure_Python_Modules
    COMMAND ${PYTHON_EXECUTABLE} -c "import py_compile; py_compile.compile('${MODULE}.py')"
    COMMENT "Byte compiling ${MODULE}.py"
    VERBATIM)
  INSTALL(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.py
    ${CMAKE_CURRENT_BINARY_DIR}/${MODULE}.pyc
    DESTINATION ${PyTrilinos_INSTALL_DIR})
  LIST(APPEND ADDITIONAL_CLEAN_FILES ${MODULE}.pyc)
  
ENDFOREACH(MODULE ${LOCAL_MODULES})

# Add the additional "make clean" files
GET_DIRECTORY_PROPERTY(clean_files ADDITIONAL_MAKE_CLEAN_FILES)
LIST(APPEND            clean_files ${ADDITIONAL_CLEAN_FILES})
LIST(REMOVE_DUPLICATES clean_files)
LIST(REMOVE_ITEM       clean_files "")
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${clean_files}")

# The NOX module employs namespaces, so include the NOX directory
IF(${PACKAGE_NAME}_ENABLE_NOX)
  ADD_SUBDIRECTORY(NOX)
ENDIF(${PACKAGE_NAME}_ENABLE_NOX)

INCLUDE(PackageAddExecutable)

INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

PACKAGE_ADD_EXECUTABLE(unittest_parser SOURCES unittest_parser.cpp parser.cpp impl_parser.cpp calls_parser.cpp list_parser.cpp mpilist_parser.cpp exception.cpp)

SET_TARGET_PROPERTIES(
  ForTrilinos_unittest_parser PROPERTIES
  LINKER_LANGUAGE CXX
  )


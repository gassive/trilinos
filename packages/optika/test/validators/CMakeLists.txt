INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)

PACKAGE_ADD_EXECUTABLE_AND_TEST(
	Validator_test
	SOURCES main.cpp)
PACKAGE_COPY_FILES_TO_BINARY_DIR(ValidatorFileCopies
  DEST_FILES
  	testFile.txt testFile2.txt testFile3.txt
  SOURCE_DIR ${PACKAGE_SOURCE_DIR}/test/validators
  EXEDEPS Validator_test
  )

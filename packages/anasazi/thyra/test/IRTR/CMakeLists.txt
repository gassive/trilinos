INCLUDE(PackageAddExecutableAndTest)

IF(Anasazi_ENABLE_Epetra)

  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    IRTRThyra_test
    SOURCES cxx_main.cpp
    ARGS 
      "--verbose --hefty --sort=SR"
      "--verbose --skinny --sort=SR"
      "--verbose --hefty --sort=LR"
      " --verbose --skinny --sort=LR" 
    COMM serial mpi
    STANDARD_PASS_OUTPUT
    )

ENDIF()

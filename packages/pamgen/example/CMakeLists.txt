INCLUDE(PackageAddExecutableAndTest)

  PACKAGE_ADD_EXECUTABLE(
    pamgen_test_driver
    SOURCES pamgen_test_driver.c getopts.C
    COMM serial mpi
    )
  PACKAGE_ADD_EXECUTABLE(
    pamgen_test_driver_l
    SOURCES pamgen_test_driver_l.c getopts.C
    COMM serial mpi
    )


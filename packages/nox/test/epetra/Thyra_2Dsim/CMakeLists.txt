
INCLUDE(PackageAddExecutableAndTest)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../utils)

ASSERT_DEFINED(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_THYRA
  NOX_ENABLE_Epetra NOX_ENABLE_EpetraExt
  NOX_ENABLE_Stratimikos Stratimikos_ENABLE_AztecOO)

IF(NOX_ENABLE_ABSTRACT_IMPLEMENTATION_THYRA AND
    NOX_ENABLE_Epetra AND NOX_ENABLE_EpetraExt AND
    NOX_ENABLE_Stratimikos AND Stratimikos_ENABLE_AztecOO)
  
  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    Thyra_NonlinearSolver_NOX_2Dsim
    SOURCES EpetraModelEval2DSim.H EpetraModelEval2DSim.C 
            Thyra_NonlinearSolver_NOX_2Dsim.C
    COMM serial mpi
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )
  
  PACKAGE_ADD_EXECUTABLE_AND_TEST(
    Thyra_2Dsim
    SOURCES EpetraModelEval2DSim.H EpetraModelEval2DSim.C Thyra_2Dsim.C
    COMM serial mpi
    NUM_MPI_PROCS 1
    PASS_REGULAR_EXPRESSION "Test passed!"
    )
  
ENDIF()

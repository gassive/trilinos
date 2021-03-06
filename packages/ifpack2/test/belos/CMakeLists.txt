
INCLUDE(PackageAddExecutableAndTest)
INCLUDE(PackageCopyFilesToBinaryDir)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../teuchos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../teuchos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../kokkos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../kokkos/NodeAPI)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../kokkos/LinAlg)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../kokkos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../tpetra/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../tpetra/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../belos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../belos/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/../../../belos/tpetra/src)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/../../../belos/tpetra/src)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR}/)

PACKAGE_CONFIGURE_FILE(${PACKAGE_NAME}_config.h)


PACKAGE_ADD_EXECUTABLE(
  tif_belos
  SOURCES belos_solve.cpp
  DEPLIBS belos tpetra kokkos teuchos
)

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_EXECUTABLE(
    tif_extprec_belos
    SOURCES belos_extprec_solve.cpp
    DEPLIBS belos tpetra kokkos teuchos
  )
ENDIF()

PACKAGE_COPY_FILES_TO_BINARY_DIR(Ifpack2BelosCopyFiles
  DEST_FILES
  test_ILUT_calore1_mm.xml test_Jacobi_calore1_mm.xml test_GS_calore1_mm.xml
  test_RILUK_calore1_mm.xml test_Cheby_calore1_mm.xml
  test_Jacobi_nos1_hb.xml test_2_Jacobi_nos1_hb.xml nos1.rsa
  test_2_ILUT_nos1_hb.xml test_2_RILUK_nos1_hb.xml
  test_4_ILUT_nos1_hb.xml test_4_RILUK_nos1_hb.xml
  test_SGS_calore1_mm.xml calore1.mtx test_pseudoblockcg_small_sym_mm.xml
  test_Jacobi_small_sym_mm.xml small_sym.mtx test_gmres_small_sym_mm.xml
  test_Diagonal_calore1_mm.xml
  test_GS_tfqmr_small_sym_mm.xml test_ILUT_tfqmr_small_sym_mm.xml
  test_tfqmr_small_sym_mm.xml test_ILUT_tfqmr_calore1_mm.xml
  5w.mtx 5w.vec 5w_bel_tif_ILUT.xml
  5w_missing_diag.mtx 5w_missing_diag_ILUT.xml
  5w_bel_tif_RILUK_0.xml 5w_bel_tif_RILUK_1.xml
  test_RILUK_tfqmr_small_sym_mm.xml
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  DEST_DIR ${CMAKE_CURRENT_BINARY_DIR}
  EXEDEPS tif_belos
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME small_gmres_belos
  ARGS "--xml_file=test_gmres_small_sym_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME small_pseudoblkcg_belos
  ARGS "--xml_file=test_pseudoblockcg_small_sym_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Diagonal_belos
  ARGS "--xml_file=test_Diagonal_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Jac_sm_belos
  ARGS "--xml_file=test_Jacobi_small_sym_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Jacobi_belos
  ARGS "--xml_file=test_Jacobi_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Jacobi_hb_belos
  ARGS "--xml_file=test_Jacobi_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Jacobi_hb_belos
  ARGS "--xml_file=test_2_Jacobi_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 2
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME ILUT_hb_belos
  ARGS "--xml_file=test_2_ILUT_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 2
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME ILUT_hb_belos
  ARGS "--xml_file=test_4_ILUT_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 4
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_hb_belos
  ARGS "--xml_file=test_2_RILUK_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 2
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_hb_belos
  ARGS "--xml_file=test_4_RILUK_nos1_hb.xml"
  COMM serial mpi
  NUM_MPI_PROCS 4
  STANDARD_PASS_OUTPUT
)

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_TEST(
    tif_extprec_belos Ifpack2BelosCopyFiles
    NAME small_qd_tfqmr_belos
    ARGS "--xml_file=test_tfqmr_small_sym_mm.xml"
    COMM serial mpi
    NUM_MPI_PROCS 1
    STANDARD_PASS_OUTPUT
  )
ENDIF()

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_TEST(
    tif_extprec_belos Ifpack2BelosCopyFiles
    NAME small_qd_GS_tfqmr_belos
    ARGS "--xml_file=test_GS_tfqmr_small_sym_mm.xml"
    COMM serial mpi
    NUM_MPI_PROCS 1
    STANDARD_PASS_OUTPUT
  )
ENDIF()

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_TEST(
    tif_extprec_belos Ifpack2BelosCopyFiles
    NAME small_qd_ILUT_tfqmr_belos
    ARGS "--xml_file=test_ILUT_tfqmr_small_sym_mm.xml"
    COMM serial mpi
    NUM_MPI_PROCS 1
    STANDARD_PASS_OUTPUT
  )
ENDIF()

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_TEST(
    tif_extprec_belos Ifpack2BelosCopyFiles
    NAME qd_ILUT_tfqmr_belos
    ARGS "--xml_file=test_ILUT_tfqmr_calore1_mm.xml"
    COMM serial mpi
    NUM_MPI_PROCS 1
    STANDARD_PASS_OUTPUT
  )
ENDIF()

IF(Ifpack2_ENABLE_QD)
  PACKAGE_ADD_TEST(
    tif_extprec_belos Ifpack2BelosCopyFiles
    NAME qd_ILUT_pbgmres_belos
    ARGS "--xml_file=5w_bel_tif_ILUT.xml"
    COMM serial mpi
    NUM_MPI_PROCS 1
    STANDARD_PASS_OUTPUT
  )
ENDIF()

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME ILUT_5w_2
  ARGS "--xml_file=5w_bel_tif_ILUT.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME ILUT_5w_no_diag
  ARGS "--xml_file=5w_missing_diag_ILUT.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_5w_0
  ARGS "--xml_file=5w_bel_tif_RILUK_0.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_5w_1
  ARGS "--xml_file=5w_bel_tif_RILUK_1.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_small_belos
  ARGS "--xml_file=test_RILUK_tfqmr_small_sym_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME Cheby_belos
  ARGS "--xml_file=test_Cheby_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME GS_belos
  ARGS "--xml_file=test_GS_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME SGS_belos
  ARGS "--xml_file=test_SGS_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME ILUT_belos
  ARGS "--xml_file=test_ILUT_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

PACKAGE_ADD_TEST(
  tif_belos Ifpack2BelosCopyFiles
  NAME RILUK_belos
  ARGS "--xml_file=test_RILUK_calore1_mm.xml"
  COMM serial mpi
  NUM_MPI_PROCS 1
  STANDARD_PASS_OUTPUT
)

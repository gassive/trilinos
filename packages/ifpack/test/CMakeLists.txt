INCLUDE(AddSubdirectories)

ADD_SUBDIRECTORIES(
  CompareWithAztecOO
  Container
  CrsRiluk
  IC
  ILU
  IHSS-SORa
  IlukGraph
  OverlappingRowMatrix
  PreconditionerFactory
  Relaxation
  SetParameters
  TestAll
  BlockCheby
  #UseTranspose
  )
IF (${PACKAGE_NAME}_ENABLE_AztecOO)
  ADD_SUBDIRECTORY(AztecOO)
ENDIF()
IF (HAVE_HYPRE)
  ADD_SUBDIRECTORY(Hypre)
ENDIF()

INCLUDE(TPLDeclareLibraries)

TPL_DECLARE_LIBRARIES( BLACS
  REQUIRED_HEADERS Bdef.h
  REQUIRED_LIBS_NAMES blacs blacsF77init
  )

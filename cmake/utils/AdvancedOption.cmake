
MACRO(ADVANCED_OPTION VARNAME)
  OPTION(${VARNAME} ${ARGN})
  MARK_AS_ADVANCED(${VARNAME})
ENDMACRO()
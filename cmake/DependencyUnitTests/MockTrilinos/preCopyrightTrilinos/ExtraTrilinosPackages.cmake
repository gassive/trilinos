
INCLUDE(PackageListHelpers)

SET( Trilinos_PACKAGES_AND_DIRS_AND_CLASSIFICATIONS
  Teko         ../preCopyrightTrilinos/teko      SS
  Stalix       ../preCopyrightTrilinos/stalix    EX
  )

#
# Disable certain packages on certain platforms.
#
# NOTE: This just makes the packages experimental 'EX' and therefore still
# allows the user to enable the package explicitly but the package will not
# get enabled implicitly.
#

PACKAGE_DISABLE_ON_PLATFORMS(Teko Windows)

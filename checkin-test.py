#!/usr/bin/env python

"""
Script for doing checkin testing of Trilinos.  Please run checkin-test.py -h
for details
"""


#
# Import commands
#


import sys
import os
import traceback

scriptsDir = os.path.abspath(os.path.dirname(sys.argv[0]))+"/cmake/python"
sys.path.insert(0, scriptsDir)

from GeneralScriptSupport import *

#
# Read in the commandline arguments
#

#print "sys.argv:", sys.argv

# Create a deep copy of the commandline arguments
cmndLineArgs = []
cmndLineArgs.extend(sys.argv)

# See if the help option is set or not
helpOpt = len( set(cmndLineArgs) & set(("--help", "-h")) ) > 0

# See if --show-defaults was set or not
showDefaultsOpt = len( set(cmndLineArgs) & set(("--show-defaults", "dummy")) ) > 0

#
# Forward the options but tee the output
#

if (not helpOpt) and (not showDefaultsOpt):
  logFileName = "checkin-test.out"
else:
  logFileName = ""

cmnd = scriptsDir+"/checkin-test-impl.py " + requoteCmndLineArgs(sys.argv[1:])

if logFileName:
  cmnd = cmnd + " 2>&1 | tee "+logFileName

# This return value is always 0 even if it fails?
rtnVal = echoRunSysCmnd(cmnd, throwExcept=False)

# Grep the output to determine success or failure
success = True
if logFileName and getCmndOutput("grep 'REQUESTED ACTIONS: PASSED' "+logFileName, True, False)=="":
  success = False
  
if success:
  sys.exit(0)
else:
  sys.exit(1)

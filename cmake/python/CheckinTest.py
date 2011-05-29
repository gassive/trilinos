
#
# ToDo:
#
#  (*) Create a TaskStatus class and use it to simplify the logic replacing
#  the simple bools.
#

#
# General scripting support
#
# NOTE: Included first to check the version of python!
#

from GeneralScriptSupport import *

from TrilinosDependencies import getTrilinosDependenciesFromXmlFile
from TrilinosDependencies import defaultTrilinosDepsXmlInFile
from TrilinosPackageFilePathUtils import *
import time


# Set the official eg/git versions!
g_officialEgVersion = "1.7.0.4"
g_officialGitVersion = "1.7.0.4"


# Read the Trilinos dependencies that will be used else where
trilinosDependencies = getTrilinosDependenciesFromXmlFile(defaultTrilinosDepsXmlInFile)


def getCommonConfigFileName():
  return "COMMON.config"


def getBuildSpecificConfigFileName(buildTestCaseName):
  return buildTestCaseName+".config"


def getInitialPullOutputFileName():
  return "pullInitial.out"


def getInitialExtraPullOutputFileName():
  return "pullInitialExtra.out"


def getInitialPullSuccessFileName():
  return "pullInitial.success"


def getModifiedFilesOutputFileName():
  return "modifiedFiles.out"


def getFinalPullOutputFileName():
  return "pullFinal.out"


def getConfigureOutputFileName():
  return "configure.out"


def getConfigureSuccessFileName():
  return "configure.success"


def getBuildOutputFileName():
  return "make.out"


def getBuildSuccessFileName():
  return "make.success"


def getTestOutputFileName():
  return "ctest.out"


def getTestSuccessFileName():
  return "ctest.success"


def getEmailBodyFileName():
  return "email.out"


def getEmailSuccessFileName():
  return "email.success"


def getInitialCommitEmailBodyFileName():
  return "commitInitialEmailBody.out"


def getInitialCommitOutputFileName():
  return "commitInitial.out"


def getFinalCommitEmailBodyFileName():
  return "commitFinalEmailBody.out"


def getFinalCommitOutputFileName():
  return "commitFinal.out"


def getCommitStatusEmailBodyFileName():
  return "commitStatusEmailBody.out"


def getPushOutputFileName():
  return "push.out"


def getExtraCommandOutputFileName():
  return "extraCommand.out"


def getHostname():
  return getCmndOutput("hostname", True)


def getEmailAddressesSpaceString(emailAddressesCommasStr):
  emailAddressesList = emailAddressesCommasStr.split(',')
  return ' '.join(emailAddressesList)


def performAnyBuildTestActions(inOptions):
  if inOptions.doConfigure or inOptions.doBuild \
    or inOptions.doTest or inOptions.doAll or inOptions.localDoAll \
    :
    return True
  return False


def performAnyActions(inOptions):
  if performAnyBuildTestActions(inOptions) or inOptions.doPull:
    return True
  return False


def doGenerateOutputFiles(inOptions):
  return performAnyActions(inOptions)


def doRemoveOutputFiles(inOptions):
  return performAnyActions(inOptions)


def assertEgGitVersionHelper(returnedVersion, expectedVersion):
  if returnedVersion != expectedVersion:
    raise Exception("Error, the installed "+returnedVersion+" does not equal the official "\
      +expectedVersion+"!  To turn this check off, pass in --no-eg-git-version-check.")
  

def assertEgGitVersions(inOptions):

  egWhich = getCmndOutput("which eg", True, False)
  if egWhich == "" or re.match(".+no eg.+", egWhich):
    raise Exception("Error, the eg command is not in your path! ("+egWhich+")")

  egVersionOuput = getCmndOutput("eg --version", True, False)
  egVersionsList = egVersionOuput.split('\n')

  if inOptions.enableEgGitVersionCheck:
    assertEgGitVersionHelper(egVersionsList[0], "eg version "+g_officialEgVersion)
    assertEgGitVersionHelper(egVersionsList[1], "git version "+g_officialGitVersion)


def assertPackageNames(optionName, packagesListStr):
  if not packagesListStr:
    return
  for packageName in packagesListStr.split(','):
    if trilinosDependencies.packageNameToID(packageName) == -1:
      validPackagesListStr = ""
      for i in range(trilinosDependencies.numPackages()):
        if validPackagesListStr != "":
          validPackagesListStr += ", "
        validPackagesListStr += trilinosDependencies.getPackageByID(i).packageName
      raise Exception("Error, invalid package name "+packageName+" in " \
        +optionName+"="+packagesListStr \
        +".  The valid package names include: "+validPackagesListStr)


def assertExtraBuildConfigFiles(extraBuilds):
  if not extraBuilds:
    return
  for extraBuild in extraBuilds.split(','):
    extraBuildConfigFile = extraBuild+".config"
    if not os.path.exists(extraBuildConfigFile):
      raise Exception("Error, the extra build configuration file " \
        +extraBuildConfigFile+" does not exit!")


def getRepoSpaceBranchFromOptionStr(extraPullFrom):
  pullFromRepoList = extraPullFrom.split(':')
  if len(pullFromRepoList) < 2:
    raise Exception("Error, the --extra-pull-from='"+pullFromRepo+"' is not a valid" \
      " <repo>:<branch> specification!")
  repo = ':'.join(pullFromRepoList[0:-1])
  branch = pullFromRepoList[-1]
  return repo + " " + branch


def executePull(inOptions, baseTestDir, outFile, pullFromRepo=None, doRebase=False):
  cmnd = "eg pull"
  if pullFromRepo:
    repoSpaceBranch = getRepoSpaceBranchFromOptionStr(pullFromRepo)
    print "\nPulling in updates from '"+repoSpaceBranch+"' ...\n"
    cmnd += " " + repoSpaceBranch
  else:
    print "\nPulling in updates from 'origin' ...\n"
    # NOTE: If you do 'eg pull origin <branch>', then the list of locally
    # modified files will be wrong.  I don't know why this is but if instead
    # you do a raw 'eg pull', then the right list of files shows up.
  if doRebase:
    cmnd += " && eg rebase --against origin/"+inOptions.currentBranch
  return echoRunSysCmnd( cmnd,
    workingDir=inOptions.trilinosSrcDir,
    outFile=os.path.join(baseTestDir, outFile),
    timeCmnd=True, returnTimeCmnd=True, throwExcept=False
    )


class Timings:
  def __init__(self):
    self.update = -1.0
    self.configure = -1.0
    self.build = -1.0
    self.test = -1.0
  def deepCopy(self):
    copyTimings = Timings()
    copyTimings.update = self.update
    copyTimings.configure = self.configure
    copyTimings.build = self.build
    copyTimings.test = self.test
    return copyTimings
  def totalTime(self):
    tt = 0.0
    if self.update > 0: tt += self.update
    if self.configure > 0: tt += self.configure
    if self.build > 0: tt += self.build
    if self.test > 0: tt += self.test
    return tt


class BuildTestCase:
  def __init__(self, name, runBuildTestCase, isDefaultBuild, extraCMakeOptions, buildIdx):
    self.name = name
    self.runBuildTestCase = runBuildTestCase
    self.isDefaultBuild = isDefaultBuild
    self.extraCMakeOptions = extraCMakeOptions
    self.buildIdx = buildIdx
    self.timings = Timings()


def setBuildTestCaseInList(buildTestCaseList_inout,
  name, runBuildTestCase, isDefaultBuild, extraCMakeOptions \
  ):
  buildTestCaseList_inout.append(
    BuildTestCase(name, runBuildTestCase, isDefaultBuild, extraCMakeOptions,
      len(buildTestCaseList_inout)))
  

def writeDefaultCommonConfigFile():

  commonConfigFileName = getCommonConfigFileName()

  if os.path.exists(commonConfigFileName):

    print "\nThe file "+commonConfigFileName+" already exists!"

  else:

    print "\nCreating a default skeleton file "+commonConfigFileName+" ..."

    commonConfigFileStr = \
      "# Fill in the minimum CMake options that are needed to build and link\n" \
      "# that are common to all builds such as the following:\n" \
      "#\n" \
      "#-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON\n" \
      "#-DBUILD_SHARED:BOOL=ON\n" \
      "#-DTPL_BLAS_LIBRARIES:PATH=/usr/local/libblas.a\n" \
      "#-DTPL_LAPACK_LIBRARIES:PATH=/usr/local/liblapack.a\n" \
      "#\n" \
      "# NOTE: Please do not add any options here that would select what pacakges\n" \
      "# get enabled or disabled.\n"

    writeStrToFile(commonConfigFileName, commonConfigFileStr)


def writeDefaultBuildSpecificConfigFile(buildTestCaseName):

  serialOrMpi = buildTestCaseName.split('_')[0]

  buildSpecificConfigFileName = getBuildSpecificConfigFileName(buildTestCaseName)

  if os.path.exists(buildSpecificConfigFileName):

    print "\nThe file "+buildSpecificConfigFileName+" already exists!"

  else:

    print "\nCreating a default skeleton file "+buildSpecificConfigFileName+" ..."

    buildSpecificConfigFileStr = \
      "# Fill in the minimum CMake options that are needed to build and link\n" \
      "# that are specific to the "+serialOrMpi+" build such as:\n" \
      "#\n" \
      "#-DBUILD_SHARED:BOOL=ON\n"

    if serialOrMpi == "MPI":
      buildSpecificConfigFileStr += \
        "#-DMPI_BASE_DIR:PATH=/usr/lib64/openmpi/1.2.7-gcc\n" \
        "#-DMPI_CXX_COMPILER:PATHNAME=/usr/lib64/openmpi/1.2.7-gcc/mpicxx\n" \
        "#-DMPI_C_COMPILER:PATHNAME=/usr/lib64/openmpi/1.2.7-gcc/mpicc\n" \
        "#-DMPI_Fortran_COMPILER:PATHNAME=/usr/lib64/openmpi/1.2.7-gcc/mpif77\n"
    elif serialOrMpi == "SERIAL":
      buildSpecificConfigFileStr += \
        "#-DCMAKE_CXX_COMPILER:PATHNAME=/usr/local/bin/g++\n" \
        "#-DCMAKE_C_COMPILER:PATHNAME=/usr/local/bin/gcc\n" \
        "#-DCMAKE_Fortran_COMPILER:PATHNAME=/usr/local/bin/gfortran\n"
    else:
      raise Exception("Invalid value for serialOrMpi="+serialOrMpi)
      
    buildSpecificConfigFileStr += \
      "#\n" \
      "# NOTE: Please do not add any options here that would change what pacakges\n" \
      "# or TPLs get enabled or disabled.\n"

    writeStrToFile(buildSpecificConfigFileName, buildSpecificConfigFileStr)


reTPlEnable = re.compile(r"-DTPL_ENABLE_.+")


reTrilinosEnableOn = re.compile(r"-DTrilinos_ENABLE_[a-zA-Z]+.+=ON")


def assertNoIllegalEnables(fileName, cmakeOption):
  success = True
  if reTPlEnable.match(cmakeOption):
    print "    ERROR: Illegal TPL enable "+cmakeOption+" in "+fileName+"!"    
    success = False
  elif reTrilinosEnableOn.match(cmakeOption):
    print "    ERROR: Illegal enable "+cmakeOption+" in "+fileName+"!"    
    success = False
  return success


def readAndAppendCMakeOptions(fileName, cmakeOptions_inout, assertNoIllegalEnablesBool):

  success = True

  if not os.path.exists(fileName):
    return

  print "\nAppending options from "+fileName+":"

  cmakeOptionsFile = open(fileName, 'r')

  for line in cmakeOptionsFile:
    if line[0] != '#':
      cmakeOption = line.strip()
      if cmakeOption == "": continue
      print "  Appending: "+cmakeOption
      if assertNoIllegalEnablesBool:
        if not assertNoIllegalEnables(fileName, cmakeOption):
          success = False
      cmakeOptions_inout.append(cmakeOption)

  return success


reModifiedFiles = re.compile(r"^[MA]\t(.+)$")


def getCurrentBranchName(inOptions, baseTestDir):
  branchesStr = getCmndOutput("eg branch", workingDir=inOptions.trilinosSrcDir)
  for branchName in branchesStr.split('\n'):
    #print "branchName =", branchName
    if branchName[0] == '*':
      currentBranch = branchName.split(' ')[1]
      #print "currentBranch =", currentBranch
      setattr(inOptions, "currentBranch", currentBranch)
      break


def getCurrentDiffOutput(inOptions, baseTestDir):
  echoRunSysCmnd(
    "eg diff --name-status origin/"+inOptions.currentBranch,
    workingDir=inOptions.trilinosSrcDir,
    outFile=os.path.join(baseTestDir, getModifiedFilesOutputFileName()),
    timeCmnd=True
    )


def extractPackageEnablesFromChangeStatus(updateOutputStr, inOptions_inout,
  enablePackagesList_inout, verbose=True ) \
  :

  modifiedFilesList = extractFilesListMatchingPattern(
    updateOutputStr.split('\n'), reModifiedFiles )

  for modifiedFileFullPath in modifiedFilesList:

    if isGlobalBuildFileRequiringGlobalRebuild(modifiedFileFullPath):
      if inOptions_inout.enableAllPackages == 'auto':
        if verbose:
          print "\nModifed file: '"+modifiedFileFullPath+"'\n" \
            "  => Enabling all Trilinos packages!"
        inOptions_inout.enableAllPackages = 'on'

    packageName = getPackageNameFromPath(trilinosDependencies, modifiedFileFullPath)
    if packageName and findInSequence(enablePackagesList_inout, packageName) == -1:
      if verbose:
        print "\nModified file: '"+modifiedFileFullPath+"'\n" \
          "  => Enabling '"+packageName+"'!"
      enablePackagesList_inout.append(packageName)


def createConfigureFile(cmakeOptions, baseCmnd, trilinosSrcDir, configFileName):

    doConfigStr = ""
  
    doConfigStr += \
      "EXTRA_ARGS=$@\n" \
      "\n" \
      +baseCmnd+ " \\\n"
  
    for opt in cmakeOptions:
      doConfigStr += opt + " \\\n"
    
    doConfigStr += \
      "$EXTRA_ARGS"

    if trilinosSrcDir:
      doConfigStr += " \\\n"+trilinosSrcDir
    
    doConfigStr += "\n"
  
    writeStrToFile(configFileName, doConfigStr)
    echoRunSysCmnd('chmod a+x '+configFileName)


def formatMinutesStr(timeInMinutes):
  return ("%.2f" % timeInMinutes) + " min"
  

def getStageStatus(stageName, stageDoBool, stagePassed, stageTiming):
  stageStatusStr = stageName + ": "
  if stageDoBool:
    if stagePassed:
      stageStatusStr += "Passed"
    else:
      stageStatusStr += "FAILED"
    stageStatusStr += " ("+formatMinutesStr(stageTiming)+")"
  else:
    stageStatusStr += "Not Performed"
  stageStatusStr += "\n"
  return stageStatusStr


def getTotalTimeBeginStr(buildTestName):
  return "Total time for "+buildTestName


def getTotalTimeLineStr(buildTestName, timeInMin):
  return getTotalTimeBeginStr(buildTestName)+" = "+str(timeInMin) + " min"


def getTimeInMinFromTotalTimeLine(buildTestName, totalTimeLine):
  if not totalTimeLine:
    return -1.0
  m = re.match(getTotalTimeBeginStr(buildTestName)+r" = (.+) min", totalTimeLine)
  if m and m.groups():
    return float(m.groups()[0])
  else:
    return -1.0


reCtestFailTotal = re.compile(r".+, ([0-9]+) tests failed out of ([0-9]+)")


def analyzeResultsSendEmail(inOptions, buildTestName,
  enabledPackagesList, cmakeOptions, startingTime, timings ) \
  :

  print ""
  print "E.1) Determine what passed and failed ..."
  print ""

  success = False

  # Determine if the update passed

  commitPassed = None
  updatePassed = None
  updateOutputExists = False

  if inOptions.doPull:

    if os.path.exists("../"+getInitialPullOutputFileName()):
      updateOutputExists = True

    if os.path.exists("../"+getInitialPullSuccessFileName()):
      print "\nThe update passed!\n"
      updatePassed = True
    elif updateOutputExists:
      print "\nThe update FAILED!\n"
      updatePassed = False
    else:
      print "\nThe update was never attempted!\n"
      updatePassed = False

  else:

    print "\nThe update step was not performed!\n"

  # Determine if the configured passed

  configurePassed = None
  configureOutputExists = False

  if inOptions.doConfigure:

    if os.path.exists(getConfigureOutputFileName()):
      configureOutputExists = True

    if os.path.exists(getConfigureSuccessFileName()):
      print "\nThe configure passed!\n"
      configurePassed = True
    elif configureOutputExists:
      print "\nThe configure FAILED!\n"
      configurePassed = False
    else:
      print "\nThe configure was never attempted!\n"
      configurePassed = False

  else:

    print "\nThe configure step was not performed!\n"

  # Determine if the build passed

  buildPassed = None
  buildOutputExists = False

  if inOptions.doBuild:

    if os.path.exists(getBuildOutputFileName()):
      buildOutputExists = True

    if os.path.exists(getBuildSuccessFileName()):
      print "\nThe build passed!\n"
      buildPassed = True
    elif buildOutputExists:
      print "\nThe build FAILED!\n"
      buildPassed = False
    else:
      print "\nThe build was never attempted!\n"
      buildPassed = False

  else:

    print "\nThe build step was not performed!\n"

  # Determine if the tests passed

  testsPassed = None
  testOutputExists = False

  if inOptions.doTest:

    if os.path.exists(getTestOutputFileName()):
      testOutputExists = True

    if not testOutputExists:

      print "\nThe tests where never even run!\n"
      testsPassed = False

    else: # testOutputExists

      testResultsLine = getCmndOutput("grep 'tests failed out of' "+getTestOutputFileName(),
        True, False)

      print "testResultsLine = '"+testResultsLine+"'"

      reCtestFailTotalMatch = reCtestFailTotal.match(testResultsLine)

      if reCtestFailTotalMatch:
        numFailedTests = int(reCtestFailTotalMatch.group(1))
        numTotalTests = int(reCtestFailTotalMatch.group(2))
        numPassedTests = numTotalTests - numFailedTests
      else:
        numTotalTests = None
        numPassedTests = None
        testsPassed = False

      if not os.path.exists(getTestSuccessFileName()):
        print "\nThe tests did not run and pass!\n"
        testsPassed = False
      elif numTotalTests == None:
        print "\nCTest was invoked but no tests were run!\n"
        testsPassed = False
      elif numTotalTests == numPassedTests:
        print "\nAll of the tests ran passed!\n"
        testsPassed = True
      else:
        print "\n"+(numTotalTests-numPassedTests)+" tests failed!\n"
        testsPassed = False

  else:

    print "\nRunning the tests was not performed!\n"

  print ""
  print "E.2) Construct the email message ..."
  print ""

  # 2.a) Construct the subject line

  overallPassed = None
  buildCaseStatus = ""
  selectedFinalStatus = False

  if inOptions.doTest and not selectedFinalStatus:
    if testOutputExists:
      if numTotalTests:
        buildCaseStatus += "passed="+str(numPassedTests)+",notpassed="+str(numFailedTests)
      else:
        buildCaseStatus += "no tests run"
      if testsPassed and numTotalTests > 0:
        overallPassed = True
      else:
        overallPassed = False
      selectedFinalStatus = True
    elif not inOptions.doBuild and not buildOutputExists:
      buildCaseStatus += "no active build exists"
      overallPassed = False
      selectedFinalStatus = True

  if inOptions.doBuild and not selectedFinalStatus:
    if buildPassed:
      buildCaseStatus += "build-only passed"
      overallPassed = True
      selectedFinalStatus = True
    elif buildOutputExists:
      buildCaseStatus += "build failed"
      overallPassed = False
      selectedFinalStatus = True

  if inOptions.doConfigure and not selectedFinalStatus:
    if configurePassed:
      buildCaseStatus += "configure-only passed"
      overallPassed = True
      selectedFinalStatus = True
    elif configureOutputExists:
      buildCaseStatus += "configure failed"
      overallPassed = False
      selectedFinalStatus = True
    else:
      buildCaseStatus += "pre-configure failed"
      overallPassed = False
      selectedFinalStatus = True

  if inOptions.doPull and not selectedFinalStatus:
    if updatePassed:
      buildCaseStatus += "update-only passed"
      overallPassed = True
      selectedFinalStatus = True
    elif updateOutputExists:
      buildCaseStatus += "update FAILED"
      overallPassed = False
      selectedFinalStatus = True

  if not selectedFinalStatus:
    raise Exception("Error, final pass/fail status not found!")

  subjectLine = "Trilinos/"+buildTestName+": "+buildCaseStatus
  if overallPassed:
    subjectLine = "passed: " + subjectLine
  else:
    subjectLine = "FAILED: " + subjectLine

  print "\nsubjectLine = '"+subjectLine+"'\n"

  success = overallPassed

  # 2.b) Construct the email body

  emailBody = subjectLine + "\n\n"

  emailBody += getCmndOutput("date", True) + "\n\n"

  emailBody += getEnableStatusList(inOptions, enabledPackagesList)
  emailBody += "Hostname: " + getHostname() + "\n"
  emailBody += "Source Dir: " + inOptions.trilinosSrcDir + "\n"
  emailBody += "Build Dir: " + os.getcwd() + "\n"
  emailBody += "\nCMake Cache Varibles: " + ' '.join(cmakeOptions) + "\n"
  if inOptions.extraCmakeOptions:
    emailBody += "Extra CMake Options: " + inOptions.extraCmakeOptions + "\n"
  if inOptions.makeOptions:
    emailBody += "Make Options: " + inOptions.makeOptions + "\n"
  if inOptions.ctestOptions:
    emailBody += "CTest Options: " + inOptions.ctestOptions + "\n"
  emailBody += "\n"
  emailBody += getStageStatus("Update", inOptions.doPull, updatePassed, timings.update)
  emailBody += getStageStatus("Configure", inOptions.doConfigure, configurePassed, timings.configure)
  emailBody += getStageStatus("Build", inOptions.doBuild, buildPassed, timings.build)
  emailBody += getStageStatus("Test", inOptions.doTest, testsPassed, timings.test)
  emailBody += "\n"

  if inOptions.doTest and testOutputExists and numTotalTests:

    fullCTestOutput = readStrFromFile(getTestOutputFileName())
    if inOptions.showAllTests:
      emailBody += fullCTestOutput
    else:
      emailBody += extractLinesAfterRegex(fullCTestOutput, r".*\% tests passed.*")

  else:

    emailBody += "\n***\n*** WARNING: There are no test results!\n***\n\n"

  endingTime = time.time()
  totalTime = (endingTime - startingTime) / 60.0

  emailBody += "\n"+getTotalTimeLineStr(buildTestName, totalTime)+"\n"

  #print "emailBody:\n\n\n\n", emailBody, "\n\n\n\n"

  writeStrToFile(getEmailBodyFileName(), emailBody)

  if overallPassed:
    echoRunSysCmnd("touch "+getEmailSuccessFileName())

  print ""
  print "E.3) Send the email message ..."
  print ""

  if inOptions.sendEmailTo:

    emailAddresses = getEmailAddressesSpaceString(inOptions.sendEmailTo)
    echoRunSysCmnd("mailx -s \""+subjectLine+"\" "+emailAddresses+" < "+getEmailBodyFileName())

  else:

    print "Not sending email because no email addresses were given!"

  # 3) Return final result

  return success


def getBuildTestCaseSummary(testCaseName, trimDown = True):
  # Get the email file
  absEmailBodyFileName = testCaseName+"/"+getEmailBodyFileName()
  if os.path.exists(absEmailBodyFileName):
    testCaseEmailStrArray = open(absEmailBodyFileName, 'r').readlines()
  else:
    testCaseEmailStrArray = None
  # Get the first line (which is the summary)
  testSummaryLine = None
  if testCaseEmailStrArray:
    summaryLine = testCaseEmailStrArray[0].strip()
    if trimDown:
      summaryLineArray = summaryLine.split(":")
      testSummaryLine = summaryLineArray[0].strip() + ": " + summaryLineArray[2].strip()
    else:
      testSummaryLine = summaryLine
  else:
    testSummaryLine = \
      "Error, The build/test was never completed!" \
      " (the file '"+absEmailBodyFileName+"' does not exist.)"
  return testSummaryLine


def getTestCaseEmailSummary(testCaseName, testCaseNum):
  # Get the email file
  absEmailBodyFileName = testCaseName+"/"+getEmailBodyFileName()
  if os.path.exists(absEmailBodyFileName):
    testCaseEmailStrArray = open(absEmailBodyFileName, 'r').readlines()
  else:
    testCaseEmailStrArray = None
  # Write the entry
  testCaseHeader = str(testCaseNum)+") "+testCaseName+" Results:"
  summaryEmailSectionStr = \
    "\n"+testCaseHeader+ \
    "\n"+getStrUnderlineStr(len(testCaseHeader))+"\n" \
    "\n"
  if testCaseEmailStrArray:
    for line in testCaseEmailStrArray:
      summaryEmailSectionStr += "  " + line
    summaryEmailSectionStr += "\n"
  else:
    summaryEmailSectionStr += \
      "Error, The build/test was never completed!" \
      " (the file '"+absEmailBodyFileName+"' does not exist.)\n"
  return summaryEmailSectionStr


def getSummaryEmailSectionStr(inOptions, buildTestCaseList):
  summaryEmailSectionStr = ""
  for buildTestCase in buildTestCaseList:
    if buildTestCase.runBuildTestCase:
      summaryEmailSectionStr += \
        getTestCaseEmailSummary(buildTestCase.name, buildTestCase.buildIdx)
  return summaryEmailSectionStr


def getEnablesLists(inOptions, baseTestDir, verbose):

  cmakePkgOptions = []
  enablePackagesList = []
    
  if inOptions.enablePackages:
    if verbose:
      print "\nEnabling only the explicitly specified packages '"+inOptions.enablePackages+"' ...\n"
    enablePackagesList = inOptions.enablePackages.split(',')
  else:
    diffOutFileName = baseTestDir+"/"+getModifiedFilesOutputFileName()
    if verbose:
      print "\nDetermining the set of packages to enable by examining "+diffOutFileName+" ...\n"
    if os.path.exists(diffOutFileName):
      updateOutputStr = open(diffOutFileName, 'r').read()
      extractPackageEnablesFromChangeStatus(updateOutputStr, inOptions,
        enablePackagesList, verbose)
    else:
      if verbose:
        print "\nThe file "+diffOutFileName+" does not exist!\n"

  for pkg in enablePackagesList:
    cmakePkgOptions.append("-DTrilinos_ENABLE_"+pkg+":BOOL=ON")

  cmakePkgOptions.append("-DTrilinos_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON")

  if inOptions.enableAllPackages == 'on':
    if verbose:
      print "\nEnabling all packages on request ..."
    cmakePkgOptions.append("-DTrilinos_ENABLE_ALL_PACKAGES:BOOL=ON")

  if inOptions.enableFwdPackages:
    if verbose:
      print "\nEnabling forward packages on request ..."
    cmakePkgOptions.append("-DTrilinos_ENABLE_ALL_FORWARD_DEP_PACKAGES:BOOL=ON")
  else:
    cmakePkgOptions.append("-DTrilinos_ENABLE_ALL_FORWARD_DEP_PACKAGES:BOOL=OFF")

  if inOptions.disablePackages:
    if verbose:
      print "\nDisabling specified packages '"+inOptions.disablePackages+"' ...\n"
    disablePackagesList = inOptions.disablePackages.split(',')
    for pkg in disablePackagesList:
      cmakePkgOptions.append("-DTrilinos_ENABLE_"+pkg+":BOOL=OFF")

  if verbose:
    print "\ncmakePkgOptions:", cmakePkgOptions

  return (cmakePkgOptions, enablePackagesList)


def runBuildTestCase(inOptions, buildTestCase, timings):

  success = True

  startingTime = time.time()

  baseTestDir = os.getcwd()

  buildTestCaseName = buildTestCase.name

  if not performAnyActions(inOptions):
    print "\nNo other actions to perform!\n"
    return success

  print "\nCreating a new build directory if it does not already exist ..."
  createDir(buildTestCaseName)

  absBuildDir = os.path.join(baseTestDir, buildTestCaseName)

  echoChDir(absBuildDir)

  try:

    print ""
    print "A) Get the CMake configure options ("+buildTestCaseName+") ..."
    print ""

    preConfigurePassed = True

    # A.1) Set the base options
  
    cmakeBaseOptions = []
  
    cmakeBaseOptions.append("-DTrilinos_ENABLE_TESTS:BOOL=ON")
  
    cmakeBaseOptions.append("-DTrilinos_TEST_CATEGORIES:STRING=BASIC")

    if inOptions.ctestTimeOut:
      cmakeBaseOptions.append(("-DDART_TESTING_TIMEOUT:STRING="+str(inOptions.ctestTimeOut)))
  
    cmakeBaseOptions.extend(buildTestCase.extraCMakeOptions)

    result = readAndAppendCMakeOptions(
      os.path.join("..", getCommonConfigFileName()),
      cmakeBaseOptions,
      True)
    if not result: preConfigurePassed = False

    reuslt = readAndAppendCMakeOptions(
      os.path.join("..", getBuildSpecificConfigFileName(buildTestCaseName)),
      cmakeBaseOptions,
      buildTestCase.isDefaultBuild)
    if not result: preConfigurePassed = False

    print "\ncmakeBaseOptions:", cmakeBaseOptions

    # A.2) Set the package enable options

    cmakePkgOptions = []
    enablePackagesList = []

    if preConfigurePassed:
      (cmakePkgOptions, enablePackagesList) = getEnablesLists(inOptions, baseTestDir, True)
  
    # A.3) Set the combined options

    cmakeOptions = []

    if preConfigurePassed:

      cmakeOptions = cmakeBaseOptions + cmakePkgOptions
    
      print "\ncmakeOptions =", cmakeOptions
    
      print "\nCreating base configure file do-configure.base ..."
      createConfigureFile(cmakeBaseOptions, "cmake", inOptions.trilinosSrcDir,
        "do-configure.base")
    
      print "\nCreating package-enabled configure file do-configure ..."
      createConfigureFile(cmakePkgOptions, "./do-configure.base", None, "do-configure")
  
    print ""
    print "B) Do the configuration with CMake ("+buildTestCaseName+") ..."
    print ""

    configurePassed = False

    if inOptions.doConfigure and not preConfigurePassed:

      print "\nSkipping configure because pre-configure failed (see above)!\n"
  
    elif inOptions.doConfigure:
  
      removeIfExists("CMakeCache.txt")

      cmnd = "./do-configure"
      if inOptions.extraCmakeOptions:
        cmnd += " " + inOptions.extraCmakeOptions

      (configureRtn, timings.configure) = echoRunSysCmnd(cmnd,
        outFile=getConfigureOutputFileName(),
        timeCmnd=True, returnTimeCmnd=True, throwExcept=False
        )

      if configureRtn == 0:
        print "\nConfigure passed!\n"
        echoRunSysCmnd("touch "+getConfigureSuccessFileName())
        configurePassed = True
      else:
        print "\nConfigure failed returning "+str(configureRtn)+"!\n"
        raise Exception("Configure failed!")
  
    else:
  
      print "\nSkipping configure on request!\n"
      if os.path.exists(getConfigureSuccessFileName()):
        print "\nA current successful configure exists!\n"
        configurePassed = True
      else:
        print "\nFAILED: A current successful configure does *not* exist!\n"
  
    print ""
    print "C) Do the build ("+buildTestCaseName+") ..."
    print ""

    buildPassed = False
  
    if inOptions.doBuild and configurePassed:
  
      cmnd = "make"
      if inOptions.makeOptions:
        cmnd += " " + inOptions.makeOptions
  
      (buildRtn, timings.build) = echoRunSysCmnd(cmnd,
        outFile=getBuildOutputFileName(),
        timeCmnd=True, returnTimeCmnd=True, throwExcept=False
        )

      if buildRtn == 0:
        print "\nBuild passed!\n"
        echoRunSysCmnd("touch "+getBuildSuccessFileName())
        buildPassed = True
      else:
        print "\nBuild failed returning "+str(buildRtn)+"!\n"
        raise Exception("Build failed!")
  
    elif inOptions.doBuild and not configurePassed:

      print "\nFAILED: Skipping the build since configure did not pass!\n"
      
    else:

      print "\nSkipping the build on request!\n"
      if os.path.exists(getBuildSuccessFileName()):
        print "\nA current successful build exists!\n"
        buildPassed = True
      else:
        print "\nFAILED: A current successful build does *not* exist!\n"
  
    print ""
    print "D) Run the tests ("+buildTestCaseName+") ..."
    print ""

    testPassed = False
  
    if inOptions.doTest and buildPassed:
  
      cmnd = "ctest"
      if inOptions.ctestOptions:
        cmnd += " " + inOptions.ctestOptions
  
      (testRtn, timings.test) = echoRunSysCmnd(cmnd,
        outFile=getTestOutputFileName(),
        timeCmnd=True, returnTimeCmnd=True, throwExcept=False
        )
  
      if testRtn == 0:
        print "\nNo tests failed!\n"
        echoRunSysCmnd("touch "+getTestSuccessFileName())
      else:
        errStr = "FAILED: ctest failed returning "+str(testRtn)+"!"
        print "\n"+errStr+"\n"
        raise Exception(errStr)
  
    elif inOptions.doTest and not buildPassed:

      print "\nFAILED: Skipping running tests since the build failed!\n"
  
    else:
  
      print "\nSkipping the tests on request!\n"

  except Exception, e:

    success = False
    printStackTrace()

  print ""
  print "E) Analyze the overall results and send email notification ("+buildTestCaseName+") ..."
  print ""

  if performAnyActions(inOptions):

    result = analyzeResultsSendEmail(inOptions, buildTestCaseName,
      enablePackagesList, cmakeOptions, startingTime, timings)
    if not result: success = False

  else:

    print "No actions performed, nothing to analyze!"

  return success


def cleanBuildTestCaseOutputFiles(runBuildTestCaseBool, inOptions, baseTestDir, buildTestCaseName):

  if runBuildTestCaseBool and not os.path.exists(buildTestCaseName):

    print "\nSkipping cleaning build/test files for "+buildTestCaseName+" because dir does not exist!\n"

  elif runBuildTestCaseBool and os.path.exists(buildTestCaseName):

    if inOptions.wipeClean:

      print "\nRemoving the existing build directory "+buildTestCaseName+" (--wipe-clean) ..."
      if os.path.exists(buildTestCaseName):
        echoRunSysCmnd("rm -rf "+buildTestCaseName)

    elif doRemoveOutputFiles(inOptions):

      echoChDir(buildTestCaseName)
      if inOptions.doConfigure or inOptions.doPull:
        removeIfExists(getConfigureOutputFileName())
        removeIfExists(getConfigureSuccessFileName())
      if inOptions.doBuild or inOptions.doConfigure or inOptions.doPull:
        removeIfExists(getBuildOutputFileName())
        removeIfExists(getBuildSuccessFileName())
      if inOptions.doTest or inOptions.doBuild or inOptions.doConfigure or inOptions.doPull:
        removeIfExists(getTestOutputFileName())
        removeIfExists(getTestSuccessFileName())
      removeIfExists(getEmailBodyFileName())
      removeIfExists(getEmailSuccessFileName())
      echoChDir("..")


def runBuildTestCaseDriver(inOptions, baseTestDir, buildTestCase, timings):

  success = True

  buildTestCaseName = buildTestCase.name

  print "\n***"
  print "*** Doing build and test of "+buildTestCaseName+" ..."
  print "***\n"
  
  if buildTestCase.runBuildTestCase:

    try:
      echoChDir(baseTestDir)
      writeDefaultBuildSpecificConfigFile(buildTestCaseName)
      result = runBuildTestCase(inOptions, buildTestCase, timings)
      if not result: success = False
    except Exception, e:
      success = False
      printStackTrace()

  else:

    print "\nSkipping "+buildTestCaseName+" build/test on request!\n"

  return success


def checkBuildTestCaseStatus(runBuildTestCaseBool, buildTestName, inOptions):

  statusMsg = None
  timeInMin = -1.0

  if not runBuildTestCaseBool:
    buildTestCaseActionsPass = True
    buildTestCaseOkayToCommit = True
    statusMsg = \
      "Test case "+buildTestName+" was not run! => Does not affect commit/push readiness!"
    return (buildTestCaseActionsPass, buildTestCaseOkayToCommit, statusMsg, timeInMin)

  if not os.path.exists(buildTestName) and not performAnyBuildTestActions(inOptions):
    buildTestCaseActionsPass = True
    buildTestCaseOkayToCommit = False
    statusMsg = "No configure, build, or test for "+buildTestName+" was requested!"
    return (buildTestCaseActionsPass, buildTestCaseOkayToCommit, statusMsg, timeInMin)

  if not os.path.exists(buildTestName):
    buildTestCaseActionsPass = False
    buildTestCaseOkayToCommit = False
    statusMsg = "The directory "+buildTestName+" does not exist!"

  emailsuccessFileName = buildTestName+"/"+getEmailSuccessFileName()
  if os.path.exists(emailsuccessFileName):
    buildTestCaseActionsPass = True
  else:
    buildTestCaseActionsPass = False

  testSuccessFileName = buildTestName+"/"+getTestSuccessFileName()
  if os.path.exists(testSuccessFileName):
    buildTestCaseOkayToCommit = True
  else:
    buildTestCaseOkayToCommit = False

  if not statusMsg:
    statusMsg = getBuildTestCaseSummary(buildTestName)

  emailBodyFileName = buildTestName+"/"+getEmailBodyFileName()
  if os.path.exists(emailBodyFileName):
    timeInMinLine = getCmndOutput("grep '"+getTotalTimeBeginStr(buildTestName)+"' " + \
      emailBodyFileName, True, False)
    timeInMin = getTimeInMinFromTotalTimeLine(buildTestName, timeInMinLine)

  return (buildTestCaseActionsPass, buildTestCaseOkayToCommit, statusMsg, timeInMin)


def getUserCommitMessageStr(inOptions):

  absCommitMsgHeaderFile = inOptions.commitMsgHeaderFile
  if not os.path.isabs(absCommitMsgHeaderFile):
    absCommitMsgHeaderFile = os.path.join(inOptions.trilinosSrcDir, absCommitMsgHeaderFile)

  print "\nExtracting commit message subject and header from the file '" \
        +absCommitMsgHeaderFile+"' ...\n"
  
  commitMsgHeaderFileStr = open(absCommitMsgHeaderFile, 'r').read()
  
  commitEmailBodyStr = commitMsgHeaderFileStr
  
  return commitEmailBodyStr


def getAutomatedStatusSummaryHeaderKeyStr():
  return "Build/Test Cases Summary"


def getAutomatedStatusSummaryHeaderStr(inOptions):
  
  commitEmailBodyStr = "\n" \
    +getAutomatedStatusSummaryHeaderKeyStr()+"\n"
  
  return commitEmailBodyStr


def getEnableStatusList(inOptions, enabledPackagesList):
  enabledStatusStr = ""
  enabledStatusStr += "Enabled Packages: " + ', '.join(enabledPackagesList) + "\n"
  if inOptions.disablePackages:
    enabledStatusStr += "Disabled Packages: " + inOptions.disablePackages + "\n"
  if inOptions.enableAllPackages == "on":
    enabledStatusStr += "Enabled all Packages\n"
  elif inOptions.enableFwdPackages:
    enabledStatusStr += "Enabled all Forward Packages\n"
  return enabledStatusStr


# Extract the original log message from the output from:
#
#   eg cat-file -p HEAD
#
# This function strips off the git-generated header info and strips off the
# trailing build/test summary data.
#
# NOTE: This function assumes that there will be at least one blank line
# between the buid/test summay data block and the original text message.  If
# there is not, this function will throw!
#
def getLastCommitMessageStrFromRawCommitLogStr(rawLogOutput):

  origLogStrList = []
  pastHeader = False
  numBlankLines = 0
  lastNumBlankLines = 0
  foundStatusHeader = False
  for line in rawLogOutput.split('\n'):
    #print "\nline = '"+line+"'\n"
    if pastHeader:
      origLogStrList.append(line)
      if line == "":
        numBlankLines += 1
      elif numBlankLines > 0:
        lastNumBlankLines = numBlankLines
        numBlankLines = 0
      if line == getAutomatedStatusSummaryHeaderKeyStr():
        foundStatusHeader = True
        break
    if line == "":
      pastHeader = True

  if foundStatusHeader:
    #print "\nlastNumBlankLines =", lastNumBlankLines
    if origLogStrList[-3] != "":
      raise Exception("Error, there must be at least one blank line before the" \
        " build/test summary block!  This should never happen!")
    origLogStrList = origLogStrList[0:-lastNumBlankLines-1]
  else:
    lastNumBlankLines = -1 # Flag we did not find status header

  return ('\n'.join(origLogStrList), lastNumBlankLines)


def getLastCommitMessageStr(inOptions):

  # Get the raw output from the last current commit log
  rawLogOutput = getCmndOutput(
    "eg cat-file -p HEAD",
    workingDir=inOptions.trilinosSrcDir
    )

  return getLastCommitMessageStrFromRawCommitLogStr(rawLogOutput)[0]


def getLocalCommitsSummariesStr(inOptions):

  # Get the list of local commits other than this one
  rawLocalCommitsStr = getCmndOutput(
    "eg log --oneline "+inOptions.currentBranch+" ^origin/"+inOptions.currentBranch,
    True,
    workingDir=inOptions.trilinosSrcDir
    )

  print \
    "\nLocal commits for this build/test group:" \
    "\n----------------------------------------" \
    "\n" \
    + rawLocalCommitsStr

  if rawLocalCommitsStr == "\n" or rawLocalCommitsStr == "":
    localCommitsExist = False
  else:
    localCommitsExist = True

  if not localCommitsExist:
    print "\nNo local commits exit!\n"

  localCommitsStr = \
    "\n" \
    "Local commits for this build/test group:\n" \
    "----------------------------------------\n"
  if localCommitsExist:
    localCommitsStr += rawLocalCommitsStr
  else:
    localCommitsStr += "No local commits exist!"
  localCommitsStr += "\n"

  return (localCommitsStr, localCommitsExist)


def getLocalCommitsSHA1ListStr(inOptions):

  # Get the raw output from the last current commit log
  rawLocalCommitsStr = getCmndOutput(
    "eg log --pretty=format:'%h' "+inOptions.currentBranch+"^ ^origin/"+inOptions.currentBranch,
    True, workingDir=inOptions.trilinosSrcDir)

  if rawLocalCommitsStr:
    return ("Other local commits for this build/test group: "
      + (", ".join(rawLocalCommitsStr.split("\n")))) + "\n"

  return ""


def checkinTest(inOptions):

  print "\n**********************************************"
  print "*** Performing checkin testing of Trilinos ***"
  print "**********************************************"

  scriptsDir = getScriptBaseDir()
  #print "\nscriptsDir =", scriptsDir

  print "\ntrilinosSrcDir =", inOptions.trilinosSrcDir

  baseTestDir = os.getcwd()
  print "\nbaseTestDir =", baseTestDir

  if inOptions.withoutDefaultBuilds:
    inOptions.withMpiDebug = False
    inOptions.withSerialRelease = False

  if inOptions.doAll:
    inOptions.doPull = True
    inOptions.doConfigure = True
    inOptions.doBuild = True
    inOptions.doTest = True

  if inOptions.localDoAll:
    inOptions.allowNoPull = True
    inOptions.doConfigure = True
    inOptions.doBuild = True
    inOptions.doTest = True

  assertEgGitVersions(inOptions)

  if inOptions.overallNumProcs:
    inOptions.makeOptions = "-j"+inOptions.overallNumProcs+" "+inOptions.makeOptions
    inOptions.ctestOptions = "-j"+inOptions.overallNumProcs+" "+inOptions.ctestOptions

  assertPackageNames("--enable-packages", inOptions.enablePackages)
  assertPackageNames("--disable-packages", inOptions.disablePackages)

  assertExtraBuildConfigFiles(inOptions.extraBuilds)

  success = True

  timings = Timings()

  subjectLine = None

  # Set up build/test cases array

  buildTestCaseList = []

  setBuildTestCaseInList( buildTestCaseList,
    "MPI_DEBUG", inOptions.withMpiDebug, True,
    [
      "-DTPL_ENABLE_MPI:BOOL=ON",
      "-DCMAKE_BUILD_TYPE:STRING=RELEASE",
      "-DTrilinos_ENABLE_DEBUG:BOOL=ON",
      "-DTrilinos_ENABLE_CHECKED_STL:BOOL=ON",
      "-DTrilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=ON"
    ]
    )

  setBuildTestCaseInList( buildTestCaseList,
    "SERIAL_RELEASE", inOptions.withSerialRelease, True,
    [
      "-DCMAKE_BUILD_TYPE:STRING=RELEASE",
      "-DTrilinos_ENABLE_DEBUG:BOOL=OFF",
      "-DTrilinos_ENABLE_CHECKED_STL:BOOL=OFF",
      "-DTrilinos_ENABLE_EXPLICIT_INSTANTIATION:BOOL=OFF"
    ]
    )

  if inOptions.extraBuilds:
    for extraBuild in inOptions.extraBuilds.split(','):
      setBuildTestCaseInList(buildTestCaseList, extraBuild, True, False, [])
  
  try:

    print "\n***"
    print "*** 0) Get the current branch name ..."
    print "***"

    getCurrentBranchName(inOptions, baseTestDir)
    print "\nCurrent branch name = " + inOptions.currentBranch

    print "\n***"
    print "*** 1) Clean old output files ..."
    print "***"
  
    if inOptions.doCommit:
      removeIfExists(getInitialCommitEmailBodyFileName())
      removeIfExists(getInitialCommitOutputFileName())

    if inOptions.doPull:
      removeIfExists(getInitialPullOutputFileName())
      removeIfExists(getInitialExtraPullOutputFileName())
      removeIfExists(getInitialPullSuccessFileName())

    removeIfExists(getFinalCommitEmailBodyFileName())
    removeIfExists(getFinalCommitOutputFileName())
    removeIfExists(getCommitStatusEmailBodyFileName())

    removeIfExists(getFinalPullOutputFileName())
    removeIfExists(getModifiedFilesOutputFileName())
    removeIfExists(getPushOutputFileName())
    if inOptions.executeOnReadyToPush:
      removeIfExists(getExtraCommandOutputFileName())

    for buildTestCase in buildTestCaseList:
      cleanBuildTestCaseOutputFiles(
        buildTestCase.runBuildTestCase, inOptions, baseTestDir, buildTestCase.name)

    print "\n***"
    print "*** 2) Commit changes before pulling updates to merge in ..."
    print "***"

    commitPassed = True # To allow logic below

    if inOptions.doCommit:
    
      try:

        print "\nNOTE: We must commit before doing an 'eg pull --rebase' ...\n"
  
        echoChDir(baseTestDir)

        print "\n2.a) Creating the commit message file ...\n"

        commitEmailBodyStr = getUserCommitMessageStr(inOptions)
        writeStrToFile(getInitialCommitEmailBodyFileName(), commitEmailBodyStr)

        print "\n2.b) Performing the initial local commit (staged and unstaged changes) ...\n"

        print \
          "\nNOTE: This is a trial commit done to allow for a safe pull.  If the  build/test\n"\
          "fails and this commit does not get erased automatically for some reason (i.e.\n" \
          "you killed the script before it was able to finish) then please remove this\n" \
          "commit yourself manually with:\n" \
          "\n" \
          "  $ eg reset --soft HEAD^\n" \
          "\n" \
          "You can then run the checkin-test.py script again to try the test, commit and\n" \
          "push again.\n"

        commitRtn = echoRunSysCmnd(
          "eg commit -a -F "+os.path.join(baseTestDir, getInitialCommitEmailBodyFileName()),
          workingDir=inOptions.trilinosSrcDir,
          outFile=os.path.join(baseTestDir, getInitialCommitOutputFileName()),
          throwExcept=False, timeCmnd=True )

        if commitRtn == 0:
          print "\nCommit passed!\n"
          commitPassed = True
        else:
          print "\nFAILED: Commit failed!\n"
          commitPassed = False

      except Exception, e:
        success = False
        commitPassed = False
        printStackTrace()

    else:

      print "\nSkipping initial commit on request ...\n"

    print "\n***"
    print "*** 3) Update the Trilinos sources ..."
    print "***"

    pullPassed = True

    doingAtLeastOnePull = inOptions.doPull

    if not doingAtLeastOnePull:

      print "\nSkipping all updates on request!\n"

    if inOptions.doCommit and not commitPassed:

      print "\nCommit failed, aborting pull!\n"
      pullPassed = False

    if doingAtLeastOnePull and pullPassed:

      #
      print "\n3.a) Check that there are no uncommited and no new unknown files before doing the pull(s) ...\n"
      #

      egStatusOutput = getCmndOutput("eg status", True, throwOnError=False,
        workingDir=inOptions.trilinosSrcDir)

      print \
        "\nOutput from 'eg status':\n" + \
        "\n--------------------------------------------------------------\n" + \
        egStatusOutput + \
        "\n--------------------------------------------------------------\n"

      repoIsClean = True

      if isSubstrInMultiLineString(egStatusOutput, "Changed but not updated"):
        print "\nERROR: There are changed unstaged uncommitted files => cannot continue!"
        repoIsClean = False

      if isSubstrInMultiLineString(egStatusOutput, "Changes ready to be committed"):
        print "\nERROR: There are changed staged uncommitted files => cannot continue!"
        repoIsClean = False

      if isSubstrInMultiLineString(egStatusOutput, "Newly created unknown files"):
        print "\nERROR: There are newly created uncommitted files => Cannot continue!"
        repoIsClean = False

      if not repoIsClean:
        print \
           "\nExplanation: In order to do a meaningful test to allow a push, all files\n" \
           "in the local repo must be committed.  Otherwise, if there are changed but not\n" \
           "committed files or new unknown files that are used in the build or the test, then\n" \
           "what you are testing is *not* what you will be pushing.  If you have changes that\n" \
           "you don't want to push, then try using 'eg stash' before you run this script to\n" \
           "stash away all of the changes you don't want to push.  That way, what you are testing\n" \
           "will be consistent with what you will be pushing.\n"
        pullPassed = False

    if doingAtLeastOnePull and pullPassed:

      # NOTE: We want to pull first from the global repo and then from the
      # extra repo so the extra repo's revisions will get rebased on top of
      # the others.  This is what you would want and expect for the remote
      # test/push process where multiple pulls may be needed before it works.

      #
      print "\n3.b) Pull updates from the global 'origin' repo ..."
      #
    
      if inOptions.doPull and pullPassed:
        echoChDir(baseTestDir)
        (updateRtn, updateTimings) = \
          executePull(inOptions, baseTestDir, getInitialPullOutputFileName())
        timings.update += updateTimings
        if updateRtn != 0:
          print "\nPull failed!\n"
          pullPassed = False
      else:
        print "\nSkipping initial pull from 'origin'!\n"

      #
      print "\n3.c) Pull updates from the extra repository '"+inOptions.extraPullFrom+"' ..."
      #

      timings.update = 0
      
      if inOptions.extraPullFrom and pullPassed:
        echoChDir(baseTestDir)
        (updateRtn, updateTimings) = \
          executePull(inOptions, baseTestDir, getInitialExtraPullOutputFileName(),
            inOptions.extraPullFrom )
        timings.update += updateTimings
        if updateRtn != 0:
          print "\nPull failed!\n"
          pullPassed = False
      else:
        print "\nSkipping extra pull from '"+inOptions.extraPullFrom+"'!\n"

    #
    print "\nDetermine overall update pass/fail ...\n"
    #

    echoChDir(baseTestDir)

    # Check for prior successful initial pull
    currentSuccessfullPullExists = os.path.exists(getInitialPullSuccessFileName())

    if inOptions.doPull:
      if pullPassed:
        print "\nUpdate passed!\n"
        echoRunSysCmnd("touch "+getInitialPullSuccessFileName())
      else:
        print "\nUpdate failed!\n"
    elif currentSuccessfullPullExists:
      print "\nA previous update was performed and was successful!"
      pullPassed = True
    elif inOptions.allowNoPull:
      print "\nNot performing update since --skip-update was passed in\n"
      pullPassed = True
    else:
      print "\nNo previous successful update is still current!"
      pullPassed = False

    # Update for current successful pull
    currentSuccessfullPullExists = os.path.exists(getInitialPullSuccessFileName())


    print "\n***"
    print "*** 4) Get the list of all the modified files ..."
    print "***"

    if pullPassed:

      getCurrentDiffOutput(inOptions, baseTestDir)

    else:

      print "\nSkipping getting list of modified files because pull failed!\n"


    print "\n***"
    print "*** 5) Running the different build/test cases ..."
    print "***"

    # Determine if we will run the build/test cases or not

    # Set runBuildCases flag
    if not performAnyBuildTestActions(inOptions):
      print "\nNot performing any build cases because no --configure, --build or --test" \
        " was specified!\n"
      runBuildCases = False
    elif inOptions.doCommit and not commitPassed:
      print "\nThe commit failed, skipping running the build/test cases!\n"
      runBuildCases = False
    elif doingAtLeastOnePull:
      if pullPassed:
        print "\nThe updated passsed, running the build/test cases ...\n"
        runBuildCases = True
      else:
        print "\nNot running any build/test cases because the update (pull) failed!\n"
        runBuildCases = False
    else:
      if inOptions.allowNoPull:
        print "\nNo pull was attemted but we are running the build/test cases anyway" \
          " because --allow-no-pull was specified ...\n"
        runBuildCases = True
      elif os.path.exists(getInitialPullSuccessFileName()):
        print "\nA previous update (pull) was successful, running build/test cases ...!\n"
        runBuildCases = True
      else:
        print "\nNot running any build/test cases because no update was attempted!\n" \
          "\nHint: Use --allow-no-pull to allow build/test cases to run without" \
          " having to do a pull first!"
        runBuildCases = False

    # Run the build/test cases

    buildTestCasesPassed = True

    if runBuildCases:

      echoChDir(baseTestDir)
  
      writeDefaultCommonConfigFile()

      print "\nSetting up to run the build/test cases:"
      for i in range(len(buildTestCaseList)):
        buildTestCase = buildTestCaseList[i]
        print str(i) + ") " + buildTestCase.name + ":",
        if buildTestCase.runBuildTestCase:
          print "Will attempt to run!"
        else:
          print "Will *not* attempt to run on request!"

      for buildTestCase in buildTestCaseList:
        buildTestCase.timings = timings.deepCopy()
        result = runBuildTestCaseDriver(
          inOptions,
          baseTestDir,
          buildTestCase,
          buildTestCase.timings
          )
        if not result:
          buildTestCasesPassed = False
          success = False


    print "\n***"
    print "*** 6) Determine overall success and commit/push readiness (and backout commit if failed) ..."
    print "***"

    okayToCommit = False
    okayToPush = False
    forcedCommitPush = False
    abortedCommitPush = False

    if inOptions.doCommitReadinessCheck or inOptions.doCommit:

      echoChDir(baseTestDir)
  
      okayToCommit = success
      subjectLine = None

      commitEmailBodyExtra = ""
      shortCommitEmailBodyExtra = ""

      (cmakePkgOptions, enabledPackagesList) = getEnablesLists(inOptions, baseTestDir, False)

      enableStatsListStr = getEnableStatusList(inOptions, enabledPackagesList)
      commitEmailBodyExtra += enableStatsListStr
      shortCommitEmailBodyExtra += enableStatsListStr

      commitEmailBodyExtra += \
        "\nBuild test results:" \
        "\n-------------------\n"

      for i in range(len(buildTestCaseList)):
        buildTestCase = buildTestCaseList[i]
        buildTestName = buildTestCase.name
        (buildTestCaseActionsPass, buildTestCaseOkayToCommit, statusMsg, timeInMin) = \
          checkBuildTestCaseStatus(buildTestCase.runBuildTestCase, buildTestName, inOptions)
        buildTestCaseStatusStr = str(i)+") "+buildTestName+" => "+statusMsg
        if not buildTestCaseOkayToCommit:
          buildTestCaseStatusStr += " => Not ready to push!"
        buildTestCaseStatusStr += " ("+formatMinutesStr(timeInMin)+")\n"
        print buildTestCaseStatusStr
        commitEmailBodyExtra += buildTestCaseStatusStr
        shortCommitEmailBodyExtra += buildTestCaseStatusStr
        #print "buildTestCaseActionsPass =", buildTestCaseActionsPass
        if not buildTestCaseActionsPass:
          success = False
        if not buildTestCaseOkayToCommit:
          okayToCommit = False
        

      # Print message if a commit is okay or not
      if okayToCommit:
        print "\nThe tests ran and all passed!\n\n" \
          "  => A COMMIT IS OKAY TO BE PERFORMED!\n"
      else:
        print "\nAt least one of the actions (update, configure, built, test)" \
          " failed or was not performed correctly!\n\n" \
          "  => A COMMIT IS *NOT* OKAY TO BE PERFORMED!\n"

      # Back out commit if one was performed and buid/test failed
      if not okayToCommit and inOptions.doCommit and commitPassed and not inOptions.forceCommitPush:
        print "\nNOTICE: Backing out the commit that was just done ...\n"
        try:
          echoRunSysCmnd("eg reset --soft HEAD^",
            workingDir=inOptions.trilinosSrcDir,
            timeCmnd=True,
            throwExcept=False )
        except Exception, e:
          success = False
          printStackTrace()
      
      # Determine if we should do a forced commit/push
      if inOptions.doCommitReadinessCheck and not okayToCommit and commitPassed \
        and inOptions.forceCommitPush \
        :
        forcedCommitPushMsg = \
          "\n***" \
          "\n*** WARNING: The acceptance criteria for doing a commit/push has *not*" \
          "\n*** been met, but a commit/push is being forced anyway by --force-commit-push!" \
          "\n***\n"
        print forcedCommitPushMsg
        okayToCommit = True
        forcedCommitPush = True

      # Determine if a push is ready to try or not

      if okayToCommit:
        if currentSuccessfullPullExists:
          print "\nA current successful pull also exists => Ready for final push!\n"
          okayToPush = True
        else:
          commitEmailBodyExtra += \
             "\nA current successful pull does *not* exist => Not ready for final push!\n" \
             "\nExplanation: In order to safely push, the local working directory needs\n" \
             "to be up-to-date with the global repo or a full integration has not been\n" \
             "performed!\n"
          print commitEmailBodyExtra
          okayToPush = False
          abortedCommitPush = True
      else:
        okayToPush = False
  
      if okayToPush:
        print "\n  => A PUSH IS READY TO BE PERFORMED!"
      else:
        print "\n  => A PUSH IS *NOT* READY TO BE PERFORMED!"

    else:

      print "\nSkipping commit readiness check on request!"
      okayToCommit = False
      okayToPush = False

  
    print "\n***"
    print "*** 7) Do final push  ..."
    print "***"

    # Attempt the final pull, commit amend, and push

    pullFinalPassed = True
    amendFinalCommitPassed = True
    pushPassed = True
    didPush = False
    localCommitSummariesStr = ""
    
    if not inOptions.doPush:
  
      print "\nNot doing the push on request (--no-push) but sending an email" \
            " about the commit/push readiness status ..."
  
      if okayToPush:
        subjectLine = "READY TO PUSH"
      else:
        subjectLine = "NOT READY TO PUSH"

    elif not okayToPush:

      print "\nNot performing push due to prior errors\n"
      pushPassed = False

    else: # inOptions.doPush and okayToPush:

      #
      print "\n7.a) Performing a final pull to make sure there are no conflicts for push ...\n"
      #
      
      if not okayToPush:

        print "\nSkippng final pull due to prior errors!\n"
        pullFinalPassed = False

      else:

        print "\nExplanation: In order to push, the local repo needs to be up-to-date\n" \
          "with the global repo or the push will not be allowed.  Therefore, a pull\n" \
          "before the push must be performed if there are updates in the global reop\n" \
          "regardless if --pull was specified or not.  Also, a rebase must be done in\n" \
          "order to get a linear history required by the hooks in the main repository.\n"

        (update2Rtn, update2Time) = \
          executePull(inOptions, baseTestDir, getFinalPullOutputFileName(), None, True)

        if update2Rtn == 0:
          print "\nFinal update passed!\n"
          pullFinalPassed = True
        else:
          print "\nFinal update failed!\n"
          pullFinalPassed = False

        if not pullFinalPassed: okayToPush = False

      #
      print "\n7.b) Amending the final commit message by appending test results ...\n"
      #

      lastCommitMessageStr = getLastCommitMessageStr(inOptions)
      #print "\nlastCommitMessageStr:\n-------------\n"+lastCommitMessageStr+"-------------\n"
      (localCommitSummariesStr, localCommitsExist) = getLocalCommitsSummariesStr(inOptions)
      #print "\nlocalCommitsExist =", localCommitsExist, "\n"
      localCommitSHA1ListStr = getLocalCommitsSHA1ListStr(inOptions)

      if not inOptions.appendTestResults:

        print "\nSkipping appending test results on request (--no-append-test-results)!\n"

      elif not okayToPush:

        print "\nSkippng appending test results due to prior errors!\n"
        amendFinalCommitPassed = False

      else:  # inOptions.appendTestResults and okayToPush
  
        print "\nAttempting to amend the final commmit message ...\n"

        try:

          # Get then final commit message
          finalCommitEmailBodyStr = lastCommitMessageStr
          finalCommitEmailBodyStr += getAutomatedStatusSummaryHeaderStr(inOptions)
          finalCommitEmailBodyStr += shortCommitEmailBodyExtra
          finalCommitEmailBodyStr += localCommitSHA1ListStr
          if forcedCommitPush:
            finalCommitEmailBodyStr += "WARNING: Forced the push!\n"
          writeStrToFile(getFinalCommitEmailBodyFileName(), finalCommitEmailBodyStr)

          # Amend the final commit message
          if localCommitsExist:

            commitAmendRtn = echoRunSysCmnd(
              "eg commit --amend" \
              " -F "+os.path.join(baseTestDir, getFinalCommitEmailBodyFileName()),
              workingDir=inOptions.trilinosSrcDir,
              outFile=os.path.join(baseTestDir, getFinalCommitOutputFileName()),
              timeCmnd=True, throwExcept=False
              )

            if commitAmendRtn == 0:
              print "\nAppending test results to last commit passed!\n"
              amendFinalCommitPassed = True
            else:
              print "\nAppending test results to last commit failed!\n"
              amendFinalCommitPassed = False

          else:

            print "\nSkipping amending last commit because there are no local commits!\n"
            
        except Exception, e:
          success = False
          amendFinalCommitPassed = False
          printStackTrace()

      if not amendFinalCommitPassed: okayToPush = False


      # Get the updates SHA1 for the amended commit but before the push!
      localCommitSummariesStr = getLocalCommitsSummariesStr(inOptions)[0]


      #
      print "\n7.c) Pushing the the local commits to the global repo ...\n"
      #

      if not okayToPush:

        print "\nNot performing push due to prior errors!\n"
        pushPassed = False

      else:
  
        print "\nAttempting to do the push ..."

        debugSkipPush = os.environ.get("CHECKIN_TEST_SKIP_PUSH","")
        #print "debugSkipPush =", debugSkipPush
        #debugSkipPush = True

        if not debugSkipPush:
          pushRtn = echoRunSysCmnd(
            "eg push origin "+inOptions.currentBranch,
            workingDir=inOptions.trilinosSrcDir,
            outFile=os.path.join(baseTestDir, getPushOutputFileName()),
            throwExcept=False, timeCmnd=True )
          None
        else:
          pushRtn = 0
  
        if pushRtn == 0:
          print "\nPush passed!\n"
          pushPassed = True
          didPush = True
        else:
          print "\nPush failed!\n"
          pushPassed = False

      if not pushPassed: okayToPush = False

  
    print "\n***"
    print "*** 8) Set up to run execute extra command on ready to push  ..."
    print "***"

    if inOptions.executeOnReadyToPush and not okayToPush:

      print "\nNot executing final command ("+inOptions.executeOnReadyToPush+") since" \
        +" a push is not okay to be performed!\n"

    elif inOptions.executeOnReadyToPush and okayToPush:

      executeCmndStr = "\nExecuting final command ("+inOptions.executeOnReadyToPush+") since" \
        +" a push is okay to be performed!\n"
      commitEmailBodyExtra += executeCmndStr
      print executeCmndStr

    else:

      print "\nNot executing final command since none was given ...\n"

  
    print "\n***"
    print "*** 9) Create and send push (or readiness status) notification email  ..."
    print "***\n"

    if inOptions.doCommitReadinessCheck:

      #
      print "\n9.a) Getting final status to send out in the summary email ...\n"
      #

      if not commitPassed:
        subjectLine = "INITIAL COMMIT FAILED"
        commitEmailBodyExtra += "\n\nFailed because initial commit failed!" \
          " See '"+getInitialCommitOutputFileName()+"'\n\n"
        success = False
      elif not pullPassed:
        subjectLine = "INITIAL PULL FAILED"
        commitEmailBodyExtra += "\n\nFailed because initial pull failed!" \
          " See '"+getInitialPullOutputFileName()+"'\n\n"
        success = False
      elif not pullFinalPassed:
        subjectLine = "FINAL PULL FAILED"
        commitEmailBodyExtra += "\n\nFailed because the final pull failed!" \
          " See '"+getFinalPullOutputFileName()+"'\n\n"
        success = False
      elif not amendFinalCommitPassed:
        subjectLine = "AMEND COMMIT FAILED"
        commitEmailBodyExtra += "\n\nFailed because the final test commit amend failed!" \
          " See '"+getFinalCommitOutputFileName()+"'\n\n"
        success = False
      elif inOptions.doPush and pushPassed and forcedCommitPush:
        subjectLine = "DID FORCED PUSH"
        commitEmailBodyExtra += forcedCommitPushMsg
        success = True
      elif inOptions.doCommit and commitPassed and forcedCommitPush:
        subjectLine = "DID FORCED COMMIT"
        commitEmailBodyExtra += forcedCommitPushMsg
      elif not buildTestCasesPassed:
        subjectLine = "FAILED CONFIGURE/BUILD/TEST"
        commitEmailBodyExtra += "\n\nFailed because one of the build/test cases failed!\n"
        success = False
      elif inOptions.doPush:
        if didPush and not forcedCommitPush:
          subjectLine = "DID PUSH"
        elif abortedCommitPush:
          subjectLine = "ABORTED COMMIT/PUSH"
          commitEmailBodyExtra += "\n\nCommit/push was never attempted since commit/push" \
          " criteria failed!\n\n"
          success = False
        else:
          subjectLine = "PUSH FAILED"
          commitEmailBodyExtra += "\n\nFailed because push failed!" \
            " See '"+getPushOutputFileName()+"'\n\n"
          success = False
      else:
        if okayToPush:
          subjectLine = "READY TO PUSH"
        else:
          subjectLine = "NOT READY TO PUSH"

      #
      print "\n9.b) Create and send out push (or readiness status) notification email ..."
      #
    
      subjectLine += ": Trilinos: "+getHostname()
  
      if inOptions.sendEmailTo:
    
        emailBodyStr = subjectLine + "\n\n"
        emailBodyStr += getCmndOutput("date", True) + "\n\n"
        emailBodyStr += commitEmailBodyExtra
        emailBodyStr += localCommitSummariesStr
        emailBodyStr += getSummaryEmailSectionStr(inOptions, buildTestCaseList)
    
        print "\nCommit status email being sent:\n" \
          "--------------------------------\n\n\n\n"+emailBodyStr+"\n\n\n\n"
  
        summaryCommitEmailBodyFileName = getCommitStatusEmailBodyFileName()
    
        writeStrToFile(summaryCommitEmailBodyFileName, emailBodyStr)
  
        emailAddresses = getEmailAddressesSpaceString(inOptions.sendEmailTo)
        if inOptions.sendEmailToOnPush and didPush:
          emailAddresses += " " + getEmailAddressesSpaceString(inOptions.sendEmailToOnPush)
        echoRunSysCmnd("mailx -s \""+subjectLine+"\" " \
          +emailAddresses+" < "+summaryCommitEmailBodyFileName)
  
      else:
  
        print "\nNot sending commit status email because --send-email-to is empty!"

    else:

      print "\nNot performing commit/push or sending out commit/push readiness status on request!"

  
    print "\n***"
    print "*** 10) Run execute extra command on ready to push  ..."
    print "***"

    if inOptions.executeOnReadyToPush and okayToPush:

      print executeCmndStr

      extraCommandRtn = echoRunSysCmnd(
        inOptions.executeOnReadyToPush,
        workingDir=baseTestDir,
        outFile=os.path.join(baseTestDir, getExtraCommandOutputFileName()),
        throwExcept=False, timeCmnd=True )

      if extraCommandRtn == 0:
        print "\nExtra command passed!\n"
      else:
        print "\nExtra command failed!\n"
        success = False

    else:

      print "\nNot executing final command ...\n"

  
    if not performAnyActions(inOptions) and not inOptions.doPush:

      print \
        "\n***\n" \
        "*** WARNING: No actions were performed!\n" \
        "***\n" \
        "*** Hint: Specify [--commit] --do-all to perform full integration update/build/test\n" \
        "*** or --push to push the commits for a previously run test!\n" \
        "***\n\n"
  
  except Exception, e:

    success = False
    printStackTrace()

  g_sysCmndInterceptor.assertAllCommandsRun()

  # Print the final status at the very end
  if subjectLine:
    print \
      "\n\n" + subjectLine + "\n\n"
  
  return success

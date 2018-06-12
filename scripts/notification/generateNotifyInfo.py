import os

'''
In order for this script to work, be sure to include the current term in the semesterDates.txt file.
The script looks for a directory in recordings with the current term name.
Formatting has to stay consistent in the semesterDates.txt file.
'''

# holds files to be sent in email
filesToEmail = []


def getRootDir():
    # path for current term and file object
    curTermPath = '/home/paol/paol-code/semesterDates.txt'
    curTermFile = open(curTermPath, 'r')

    # gets current term from semesterDates.txt
    lines = curTermFile.readlines()
    termLine = lines[len(lines)-1]
    term = termLine.split(":")[0]
    curTermFile.close()

    # current recording directory
    rootDir = '/home/paol/recordings/readyToUpload/' + term
    return rootDir


def getLogDirs(rootDir):
    # finds directories of log files
    logDirs = []
    for dirname, dirnames, filenames in os.walk(rootDir):
        # adding all log subdirectories in an array
        for subdirname in dirnames:
            if subdirname == 'logs':
                logDirs.append(os.path.join(dirname, subdirname))
    return logDirs


def getINFOPath(rootDir):
    # finds paths to INFO files
    infoPaths = []
    for dirname, dirnames, filenames in os.walk(rootDir):
        # adding all INFO file paths in an array
        for filename in filenames:
            if filename == 'INFO':
                infoPaths.append(os.path.join(dirname, filename))
    return infoPaths


def writeFromINFO(logDir, INFOPath):
    # check to see if ffmpeg.log file exists
    if os.path.isfile(logDir + '/' + 'ffmpeg.log'):
        ffempegExists = True
    else:
        ffempegExists = False

    # open INFO file for reading
    INFOFile = open(INFOPath, 'r')
    # open notifyInfo.txt for writing
    notifyInfoFile = open(logDir + '/' + 'notifyInfo.txt', 'w')

    # read lines from INFO file
    lines = INFOFile.readlines()

    # file header copied from INFO file
    notifyInfoFile.write('========== INFO FILE INFORMATION ==========\n')
    for line in lines:
        notifyInfoFile.write(line)
    notifyInfoFile.write('========== END OF INFO FILE ==========\n\n')

    # status of ffmpeg.log file, reads from log if file is present
    takeLines = []
    if ffempegExists:
        status = 'FOUND\n'
        # checks for errors in log file
        errorsPresent = findErrors(logDir + '/' + 'ffmpeg.log')

        # opens log file for reading and pulls lines
        if errorsPresent:
            notifyInfoFile.write('Status of errors: FOUND\n')

            # adds file to email list if errors are present
            filesToEmail.append(logDir + '/' + 'notifyInfo.txt')

            FFMFile = open(logDir + '/' + 'ffmpeg.log', 'r')
            takeLines = FFMFile.readlines()
            FFMFile.close()
        else:
            notifyInfoFile.write('Status of errors: NOT FOUND\n')
    else:
        # adds file to email list if ffmpeg.log file is not found
        filesToEmail.append(logDir + '/' + 'notifyInfo.txt')
        status = 'NOT FOUND\n'

    # writes status of ffmpeg.log file and any information if errors were found
    notifyInfoFile.write('Status of ffmpeg.log: ' + status + '\n')
    notifyInfoFile.write('========== FFMPEG.LOG FILE INFORMATION ==========')
    for line in takeLines:
        notifyInfoFile.write(line)
    if not takeLines:
        notifyInfoFile.write('\n')
    notifyInfoFile.write('========== END OF FFMPEG.LOG FILE ==========\n')

    # closing files
    INFOFile.close()
    notifyInfoFile.close()


def findErrors(file):
    # reads file and checks for error keywords
    textFile = open(file, "r")
    lines = textFile.readlines()
    x = 0
    for i in lines:
        # changes file to be lowercase
        theLine = lines[x].lower()
        if "error" in theLine:
            return True
        if "critical" in theLine:
            return True
        if "abort" in theLine:
            return True
        if "failed" in theLine:
            return True
        if "core dump" in theLine:
            return True
        if "warning" in theLine:
            return True
        x += 1
    textFile.close()
    return False


def sumFilePaths():
    # collects files to be sent to email, writes to a file
    newFile = open('/home/paol/recordings/notifyInfoAll.txt', 'w+')
    for pathToWrite in filesToEmail:
        newFile.write(pathToWrite + '\n')
    newFile.close()


def genFiles(logDirs, infoPaths):
    # creates all notify files
    itr = 0
    for dir in logDirs:
        writeFromINFO(logDirs[itr], infoPaths[itr])
        itr = itr + 1


def refreshFiles(logDirs):
    # recreates notifyInfo.txt file for new information
    for dirs in logDirs:
        newFile = open(dirs + '/' + 'notifyInfo.txt', 'w+')
        newFile.close()


def printStuff(dirs):
    # for testing purposes
    for stuff in dirs:
        print stuff


def main():
    rootDir = getRootDir()
    logDirs = getLogDirs(rootDir)
    infoPaths = getINFOPath(rootDir)
    refreshFiles(logDirs)
    genFiles(logDirs, infoPaths)
    sumFilePaths()


main()

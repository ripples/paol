import subprocess
import shlex
import os
import signal

path = "/home/paol/recordings/readyToUpload/2018/test1/06-05-2018--11-55-13/"

'''
Opens a log file at a given path
Reads through the file and looks for certian error key words
If found, returns True, if not, returns False
'''
def parseFile():
    textFile = open(path + "logs/ffmpeg.log", "r")
    lines = textFile.readlines()
    x = 0
    for i in lines:
	#changes file to be lowercase
        theLine = lines[x].lower()
        if "error" in theLine:
            print("Found")
            return True
        if "critical" in theLine:
            print("Found")
            return True
        if "abort" in theLine:
            print("Found")
            return True
        if "core dump" in theLine:
            print("Found")
            return True
	if "warning" in theLine:
	    print("Found")
	    return True
        x += 1

    textFile.close()
    return False


def main():
    message = ''
    output = parseFile()

    if output:
        x = 0
        fileIn = open(path + "logs/ffmpeg.log", "r")
        error = fileIn.readlines()
        for i in error:
            message += i
	#replaces ' ' with '_' because python hates white space
        message = message.replace(" ", "_")
	#command to be sent to shell, takes command line arguements to be sent to emailerror.py
        cmd = "python /home/paol/paol-code/scripts/notification/emailerror.py paolDev1 TEST testdate " + "'" + message + "'"
	#sends command to shell
        cmd = shlex.split(cmd)
        process = subprocess.Popen(cmd)
        (result, err) = process.communicate()
    	fileIn.close()

main()

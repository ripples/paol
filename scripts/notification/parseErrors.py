import subprocess
import shlex
import os
import signal

path = "/home/paol/recordings/readyToUpload/SUMMER18/COMPTEST3/06-07-2018--12-17-38/"


def parseFile():
    textFile = open(path + "logs/ffmpeg.log", "r")
    lines = textFile.readlines()
    x = 0
    for i in lines:
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
        message = message.replace(" ", "_")
        cmd = "python /home/paol/paol-code/scripts/notification/emailerror.py paolDev1 TEST testdate " + "'" + message + "'"
        cmd = shlex.split(cmd)
        process = subprocess.Popen(cmd)
        (result, err) = process.communicate()
    fileIn.close()

main()

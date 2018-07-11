import subprocess
import shlex
import os
import signal

path = "/home/paol/recordings/notifyInfoAll.txt"


def parseInfos():
    message = ''
    allInfoFile = open(path, "r")
    lines = allInfoFile.readlines()
    for line in lines:
        line = line.replace("\n","")
        print line
        infoFile = open(line, "r")
        info = infoFile.readlines()
        for text in info:
            message += text
        infoFile.close()
        message += "\n\n"
    allInfoFile.close()
    return message


def main():
    message = ''
    output = parseInfos()

    if not output:
	return

    for i in output:
        message += i
    # replaces ' ' with '_' because python hates white space
    message = message.replace(" ", "_")

    # command to be sent to shell, takes command line arguements to be sent to emailerror.py
    cmd = "python /home/paol/paol-code/scripts/notification/emailerror.py " + "'" + message + "'"

    # sends command to shell
    cmd = shlex.split(cmd)
    process = subprocess.Popen(cmd)
    (result, err) = process.communicate()


main()

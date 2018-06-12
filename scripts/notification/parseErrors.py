import subprocess
import shlex
import os
import signal

path = "/home/paol/recordings/notifyInfoAll.txt"

'''
Opens a log file at a given path
Reads through the file and looks for certian error key words
If found, returns True, if not, returns False
'''
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
'''


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
        message+= "\n\n"
    allInfoFile.close()
    return message

def main():
    message = ''
    output = parseInfos()

    for i in output:
        message += i
    # replaces ' ' with '_' because python hates white space
    message = message.replace(" ", "_")
    # command to be sent to shell, takes command line arguements to be sent to emailerror.py
    cmd = "python /home/paol/paol-code/scripts/notification/emailerror.py TEST " + "'" + message + "'"
    # sends command to shell
    cmd = shlex.split(cmd)
    process = subprocess.Popen(cmd)
    (result, err) = process.communicate()


main()

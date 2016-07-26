#!/bin/bash

# Used to check if the machine is working and if it's not, send an email. - File Creator.

# Make sure that there is exactly one argument, path to code directory
if [ "$#" -ne 1 ]; then
	echo "Usage: ./runningCheck.sh <path to code directory>"
	exit
fi

# Set the first argument as the code directory to create file
Dir=$1

# The fields to read from the check configuration file
user=
host=
rmt_runcheck=

# The location of the check configuration file
config=$Dir/paol-code/uploadConfig.txt

# Read each line in the config file, assuming they are of format "blah: blah", and set the fields
while read a b; do
	if [ "$a" = "user:" ]; then
		user=$b
	elif [ "$a" = "host:" ]; then
		host=$b
        elif [ "$a" = "rmt_runcheck:" ]; then
		rmt_runcheck=$b
	fi
done < $config

echo "User: $user"
echo "Host: $host"
echo "Remote checkpoint folder: $rmt_runcheck"

# Exit if one of the parameters is not specified
if [ -z "$user" ] || [ -z "$host" ] || [ -z "$rmt_runcheck" ]; then
	echo "Running/File check configuration file has the wrong format"
	exit 1
fi

# Set hostname and date to provide which machine it is and create file in the server
machine=$HOSTNAME
now="$(date +'%T-%m-%d-%Y')"

# Create file
sshid=/home/paol/.ssh/id_rsa
ssh -i $sshid $user@$host touch /var/lock/manic.lck
ssh -i $sshid $user@$host mkdir -p $rmt_runcheck/$machine
ssh -i $sshid $user@$host touch $rmt_runcheck/$machine/$now.txt
ssh -i $sshid $user@$host rm /var/lock/manic.lck

exit

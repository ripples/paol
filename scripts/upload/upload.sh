#!/bin/bash

user=
host=
rmt_upload=

config=/home/paol/paol-code/uploadConfig.txt
while read a b; do
	if [ "$a" = "user:" ]; then
		user=$b
	elif [ "$a" = "host:" ]; then
		host=$b
	elif [ "$a" = "rmt_upload:" ]; then
		rmt_upload=$b
	fi
done < $config

echo "User: $user"
echo "Host: $host"
echo "Remote upload folder: $rmt_upload"

# Exit if one of the parameters is not specified
if [ -z "$user" ] || [ -z "$host" ] || [ -z "$rmt_upload" ]; then
	echo "Upload configuration file has the wrong format"
	exit 1
fi

outDir=$1
sem=$(basename $(dirname $(dirname $outDir)))
class=$(basename $(dirname $outDir))

uploaded=/home/paol/recordings/uploaded

#upload files
sshid=/home/paol/.ssh/id_rsa
ssh -i $sshid $user@$host touch /var/lock/manic.lck
ssh -i $sshid $user@$host mkdir -p $rmt_upload/$sem/$class
rsync -avz -e "ssh -i $sshid" $outDir $user@$host:$rmt_upload/$sem/$class
STATUS=$?
ssh -i $sshid $user@$host rm /var/lock/manic.lck

if [ $STATUS = 0 ]; then
  echo "Uploaded lecture $(basename $outDir) from $class in $sem, moving to uploaded folder"
  mkdir -p $uploaded/$sem/$class
  mv $outDir $uploaded/$sem/$class
else
  echo "rsync failure"
fi

exit $STATUS
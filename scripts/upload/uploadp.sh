#!/bin/bash

# Make sure that there is exactly one argument, the path of the lecture to upload
if [ "$#" -ne 1 ]; then
	echo "Usage: ./upload.sh <path of lecture to upload>"
	exit
fi
# Set the first argument as the lecture directory to upload
outDir=$1

# The fields to read from the upload configuration file
user=
host=
rmt_upload=
rmt_upload2=

# The location of the upload configuration file
config=/home/paol/paol-code/uploadConfig.txt

# Read each line in the config file, assuming they are of format "blah: blah", and set the fields
while read a b; do
	if [ "$a" = "user:" ]; then
		user=$b
	elif [ "$a" = "host:" ]; then
		host=$b
	elif [ "$a" = "rmt_upload:" ]; then
		rmt_upload=$b
	elif [ "$a" = "rmt_upload2:" ]; then
		rmt_upload2=$b
	fi
done < $config

echo "User: $user"
echo "Host: $host"
echo "Remote upload folder: $rmt_upload"
echo "Remote upload folder: $rmt_upload2"

# Exit if one of the parameters is not specified
if [ -z "$user" ] || [ -z "$host" ] || [ -z "$rmt_upload" ] || [ -z "$rmt_upload2" ]; then
	echo "Upload configuration file has the wrong format"
	exit 1
fi

# If outDir ends in a slash, remove it
if [ ${outDir: -1} = "/" ]; then
	outDir=${outDir:0:-1}
fi
# Get the semester and class name from the given lecture path
sem=$(basename $(dirname $(dirname $outDir)))
class=$(basename $(dirname $outDir))

#upload files
sshid=/home/paol/.ssh/id_rsa
ssh -i $sshid $user@$host touch /var/lock/manic.lck
ssh -i $sshid $user@$host mkdir -p $rmt_upload/$sem/$class
# scp -r $outDir $user@$host:$rmt_upload/$sem/$class
zip -r $outDir $outDir
outZip=$outDir
outZip+=".zip"
echo "Uploading zip $outZip to present""
curl -X POST \
  http://localhost:3001/upload/COMPSCI311/lecture-zip \
  -H 'cache-control: no-cache' \
  -H 'content-type: multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW' \
  -H 'postman-token: d02cc18e-b597-ecf4-f3d5-b5f02a055628' \
  -F file=@$outZip
#the following is for files without video and NEEDS TESTING
# rsync -avz -e "ssh -i $sshid" --exclude *.mp4 $outDir $user@$host:$rmt_upload/$sem/$class 
STATUS=$?
ssh -i $sshid $user@$host rm /var/lock/manic.lck

# The path where the uploaded lectures should go
uploaded=/home/paol/recordings/uploaded

if [ $STATUS = 0 ]; then
  echo "Uploaded lecture $(basename $outDir) from $class in $sem, moving to uploaded folder"
  mkdir -p $uploaded/$sem/$class
  mv $outDir $uploaded/$sem/$class
  mv $outZip $uploaded/$sem/$class
else
  echo "upload failure"
fi

exit $STATUS

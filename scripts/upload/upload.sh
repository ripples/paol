#!/bin/bash

user=
host=

config=/home/paol/paol-code/uploadConfig.txt
while read a b; do
	if [ "$a" = "user:" ]; then
		user=$b
	elif [ "$a" = "host:" ]; then
		host=$b
	fi
done < $config

echo "User: $user"
echo "Host: $host"

if [ -z "$user" ] || [ -z "$host" ]; then
	echo "Upload configuration file has the wrong format"
	exit 1
fi

outDir=$1
sem=$(basename $(dirname $(dirname $outDir)))
class=$(basename $(dirname $outDir))

uploaded=/home/paol/recordings/uploaded
sshid=/home/paol/.ssh/id_rsa
mediaDirTag=$(curl http://$host/api/config | grep mediaDirectory)
#trim {"mediaDirectory":"/ and "/}
mediaDirTag=${mediaDirTag:20:${#mediaDirTag}-22}
echo $mediaDirTag
rmt_upload_fldr=/data1/www/$mediaDirTag
echo $rmt_upload_fldr

#upload files
ssh -i $sshid $user@$host touch /var/lock/manic.lck
ssh -i $sshid $user@$host mkdir -p $rmt_upload_fldr/$sem/$class
rsync -avz -e "ssh -i $sshid" $outDir $user@$host:$rmt_upload_fldr/$sem/$class
STATUS=$?
ssh -i $sshid $user@$host rm /var/lock/manic.lck

if [ $STATUS = 0 ]; then
  echo "Uploaded lecture $(basename $outDir) from $class in $sem, moving to uploaded folder"
  mkdir -p $uploaded/$sem/$class
	#touch local lock?
  mv $outDir $uploaded/$sem/$class
  #remove local lock?
else
  echo "rsync failure"    
fi

exit $STATUS
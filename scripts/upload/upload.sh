#!/bin/bash

#localMachName = name of local machine (ex. cap140)
localMachName=cap140
user=webmanic
host=present.cs.umass.edu

#above could be moved to a config file

outDir=$1
sem=$(basename $(dirname $(dirname $outDir)))
class=$(basename $(dirname $outDir))

uploaded=/home/paol/recordings/uploaded
sshid=/home/paol/.ssh/id_rsa
#rmt_upload_fldr=/data1/recording/$localMachName/recordings/uploaded
mediaDirTag=`curl http://$host/api/config | grep mediaDirectory`
#trim {"mediaDirectory":"\ and "\}
mediaDirTag=${mediaDirTag:20:${#mediaDirTag}-23}
echo $mediaDirTag
rmt_upload_fldr=/data1/www/$mediaDirTag/uploaded
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
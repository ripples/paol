#!/bin/bash

#############
## Config ###
#############

#semester the course is being taught(Fall13)
term=$1
#Course Number(COMP115)
crs=$2
#length of recording in seconds
#starts 1 min before; normal class dur. 50mins; 5 mins longer;
#this usually adds up to 3360 seconds
dur=$3

#locks the process so nothing else can kill it; may not be necessary
#find what r is..
lck="/var/lock/rmanic.lck"

while [ -f $lck ]; do
    echo "locked...waiting $dur"
    dur=$((dur -1))
    if [ $dur -le 0 ]; then
	echo "waited to long"
	exit 0
    fi
    sleep 1
done

touch /var/lock/manic.lck
touch $lck
#//end lock

#location for the recordings to be stored
#record=/recordings/raw
record=~/recordings/raw
mkdir -p $record

pth=$record/$term/$crs/`date +%m-%d-%Y--%H-%M-%S`
pthCam=$pth/video.ogv
#log="sudo tee $pth/main.log" maynotbenecesary
log="tee $pth/main.log"

#location of capture scripts; hw:microphone; number corresponds to mic and cam

cameraFile="/home/paol/paol-code/cameraSetup.txt"
line=$(grep Video $cameraFile)
vidNum=${line:0:1}
vidCam="/home/paol/paol-code/scripts/capture/videoCapture /dev/video$vidNum hw:$vidNum"
dataCam="/home/paol/paol-code/captureProcessCode/camCap"

##################
## PRE RECORDING ##
###################

## create path
mkdir -p $pth
mkdir -p $pth/wboard
mkdir -p $pth/computer
cd $pth

####################
## Record ##########
####################
vgaCount=0 #counter for vga feeds
camCount=0 #counter for wboard feeds
pids="" #pids for each camCap process
while read num type
do
	if [ "$type" = "Whiteboard" ]; then
		$dataCam $pth/wboard/ $dur $type $num $camCount &> $pth/wboardDataCam$camCount.log &
		pids="$pids $!" #add new camCap pid
		camCount=$(($camCount+1)) #increment counter
	fi
	if [ "$type" = "VGA2USB" ]; then
		$dataCam $pth/computer/ $dur $type $num $vgaCount &> $pth/vgaDataCam$vgaCount.log &
		pids="$pids $!" #add new camCap pid
		vgaCount=$(($outNo+1)) #increment counter
	fi
done < $cameraFile #read from camera file

echo "VGA and whiteboard PIDs:" $pids

#$dataCam $pth/computer/ $pth/wboard/ $dur $cameraFile &> $pth/dataCam.log &

#dataCamPID=$!

$vidCam $dur $pth/video.mpeg &> $pth/vidCam.log &

vidCamPID=$!

#record information
echo "[course]" >> $pth/INFO
echo "id: $crs" >> $pth/INFO
echo "term: $term" >> $pth/INFO
echo "" >> $pth/INFO
echo "[pres]" >> $pth/INFO
echo "start: `date +%Y,%m,%d,%k,%M,%S`" >> $pth/INFO
echo "duration: $dur" >>$pth/INFO

echo "waiting for processes to finish"
#wait $dataCamPID
wait $pids
wait $vidCamPID

echo "Finished" | $log

rm -rf /var/lock/manic.lck
rm -rf $lck

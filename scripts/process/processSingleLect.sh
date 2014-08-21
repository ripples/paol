#!/bin/bash

lck="/var/lock/manic.lck";

if [ -f $lck ]; then
    echo "Locked"
    exit 0
fi

next=$1

touch $lck

ffmpeg="/usr/local/bin/ffmpeg"
echo "running process"
echo $(basename $next)
#the following sets up the directory for the processed lecture
outClassDir=/home/paol/recordings/readyToUpload/$(basename $(dirname $(dirname $next)))/$(basename $(dirname $next))
outDir="$outClassDir/$(basename $next)" 
presName="$(basename $next)"
echo $outClassDir
echo $outDir

#the following sets up the directory to move the processed raw data
# to after processing
moveClassDir=/home/paol/recordings/processed/$(basename $(dirname $(dirname $next)))/$(basename $(dirname $next))
moveDir="$moveClassDir/$(basename $next)" 
presName="$(basename $next)"
echo $moveClassDir
echo $moveDir

echo "Will output to: $outDir"

echo "Making dirs"
mkdir -p $outClassDir
mkdir $outDir
mkdir $outDir/logs
mkdir $outDir/computer
mkdir $outDir/whiteboard
mkdir -p $moveClassDir
mkdir $moveDir

echo "Copying logs"
cp $next/*.log $outDir/logs/

echo "Copying video"
#must install codecs before this can work
#if [ "$(ls -A $next/video.mpeg)" ]; then
#    ffmpeg -i $next/video.mpeg -an -pass 1 -vcodec libx264 -vpre slowfirstpass -b 200k -threads 8 -f rawvideo -y /dev/null 
#    ffmpeg -i $next/video.mpeg -an -pass 2 -vcodec libx264 -vpre normal -b 200k -threads 8 -y $outDir/video.mp4
#    ffmpeg -i $next/video.mpeg -an -vcodec libtheora -threads 8 -y $outDir/video.ogv
#fi
#cp $next/video.mpeg $outDir
#cp $next/video.mp4 $outDir
for vid in *.mp4;
do
    cp $next/$vid $outDir
    ffmpeg -i $next/$vid -c:v libvpx -crf 10 -b:v 200k -c:a libvorbis $outDir/${vid/%.mp4}.webm
done
 
echo "Copying INFO"
cp $next/INFO $outDir/INFO

procComp=/home/paol/paol-code/captureProcessCode/processCOMP
procWB=/home/paol/paol-code/captureProcessCode/processWB
#args=" -O $outDir/ -I $outDir/INFO"

if [ "$(find $next/computer -iname vgaTwoUsbIn000000*)" ]; then
    screenDir="$next/computer/"
    for screenFirst in $(ls $next/computer/vgaTwoUsbIn000000*)
    do
	echo screenFirst
	firstIm="$(basename $screenFirst)"
	echo "Will process screen: $screenDir $firstIm"
	$procComp $next/computer/ $firstIm $outDir/computer/ &> $outDir/logs/procComp.log
    done
else
    echo "No computer to process"
fi

if [ "$(find $next/wboard -iname cameraIn000000*)" ]; then
    screenDir="$next/wboard/"
    for screenFirst in $(ls $next/wboard/cameraIn000000*)
    do
	echo screenFirst
	firstIm="$(basename $screenFirst)"
	echo "Will process screen: $screenDir $firstIm"
	$procWB $next/wboard/ $firstIm $outDir/whiteboard/ &> $outDir/logs/procCam.log
    done
else
    echo "No whiteboard to process"
fi

#move processed files to processed directory
mv $next $moveClassDir

rm -rf $lck

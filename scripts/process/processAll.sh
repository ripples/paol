#!/bin/bash

lck="/var/lock/manic.lck";

#if [ -f $lck ]; then
#    echo "Locked"
#    exit 0
#fi

for semester in $(ls ~/recordings/raw)
do
    echo $semester
    for course in $(ls ~/recordings/raw/$semester)
    do
	echo $course
	for lecture in $(ls ~/recordings/raw/$semester/$course)
	do
	    echo $lecture
	    ~/paol-code/scripts/process/processSingleLect.sh "/home/paol/recordings/raw/$semester/$course/$lecture"
	done
    done
done
next=( $(find ~/recordings/raw/Summer13 -iname "*-*-*--*-*-*" -type d) )
next=${next[0]}
echo $next
#if [ "$1" == "" ]; then
#    next=( $(find ~/recordings/raw/Summer13 -iname "*-*-*--*-*-*" -type d) )
#    next=${next[0]}
#    echo $next
#else
#    next=$1;
#fi

#if [ "$(ls -A $next/computer/*)" ]; then
#    screenDir="$next/computer/"
#    for screenFirst in $(ls $next/computer/vgaTwoUsbIn000000*)
#    do
#	echo screenFirst
#	firstIm="$(basename $screenFirst)"
#	echo "Will process screen: $screenDir $firstIm"
#	$procComp $next/computer/ $firstIm $outDir/computer/ &> $outDir/logs/procComp.log
#    done
#else
#    echo "No computer to process"
#fi


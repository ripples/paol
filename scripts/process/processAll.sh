#!/bin/bash

lck="/var/lock/manic.lck";

#if [ -f $lck ]; then
#    echo "Locked"
#    exit 0
#fi

for semester in $(ls /home/paol/recordings/raw)
do
    echo $semester
    for course in $(ls /home/paol/recordings/raw/$semester)
    do
	echo $course
	for lecture in $(ls /home/paol/recordings/raw/$semester/$course)
	do
	    echo $lecture
	    /home/paol/paol-code/scripts/process/processSingleLect.sh "/home/paol/recordings/raw/$semester/$course/$lecture"
	done
    done
done

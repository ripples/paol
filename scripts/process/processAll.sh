#!/bin/bash

lck="/var/lock/manic.lck";

#if [ -f $lck ]; then
#    echo "Locked"
#    exit 0
#fi

echo "Processing and uploading raw lectures"
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

echo "Uploading readyToUpload lectures"
for semester in $(ls /home/paol/recordings/readyToUpload)
do
    echo $semester
    for course in $(ls /home/paol/recordings/readyToUpload/$semester)
    do
	echo $course
	for lecture in $(ls /home/paol/recordings/readyToUpload/$semester/$course)
	do
	    echo $lecture
	    /home/paol/paol-code/scripts/upload/upload.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
		echo
	done
    done
done
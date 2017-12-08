echo "Uploading readyToUpload lectures"
url=$1
for semester in $(ls /home/paol/recordings/readyToUpload)
do
    echo $semester
    for course in $(ls /home/paol/recordings/readyToUpload/$semester)
    do
	echo $course
	for lecture in $(ls /home/paol/recordings/readyToUpload/$semester/$course)
	do
	    echo $lecture
	    /home/paol/paol-code/scripts/upload/compressVideo.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
	    /home/paol/paol-code/scripts/upload/createThumbnails.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
	    /home/paol/paol-code/scripts/upload/uploadcurl.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture" "$url"
		echo
	done
    done
done

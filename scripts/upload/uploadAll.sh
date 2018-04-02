echo "Uploading readyToUpload lectures"
url=$1
# delete raw folder
rm -r /home/paol/recordings/readyToUpload/raw/
for semester in $(ls /home/paol/recordings/readyToUpload)
do
    echo "Semester:	$semester"
    for course in $(ls /home/paol/recordings/readyToUpload/$semester)
    do
	echo ">Course:	$course"
	for lecture in $(ls /home/paol/recordings/readyToUpload/$semester/$course)
	do
	    echo ">>Lecture:	$lecture"
	    /home/paol/paol-code/scripts/upload/compressVideo.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
	    /home/paol/paol-code/scripts/upload/createThumbnails.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
#	    /home/paol/paol-code/scripts/upload/upload.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
	    /home/paol/paol-code/scripts/upload/uploadcurl.sh "/home/paol/recordings/readyToUpload/$semester/$course/$lecture"
		echo
	done
    done
done

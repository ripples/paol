echo "Uploading readyToUpload lectures"
lecturePath=$1/recordings/readyToUpload
codePath=$1
echo $lecturePath
for semester in $(ls $lecturePath)
do
    echo $semester
    for course in $(ls $lecturePath/$semester)
    do
	echo $course
	for lecture in $(ls $lecturePath/$semester/$course)
	do
	    echo $lecture
	    echo "$lecturePath/$semester/$course/$lecture"
	    $codePath/paol-code/scripts/upload/createThumbnails.sh "$lecturePath/$semester/$course/$lecture"
	    $codePath/paol-code/scripts/upload/uploadPortable.sh "$lecturePath/$semester/$course/$lecture" $codePath
		echo
	done
    done
done

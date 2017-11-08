#!/bin/bash

script_mode=$2
info_file=$1

notificationInfo() {

	admin=$(awk '$0 == "[ADMIN]" {i=1;next};i && i++ <= 1' "$1")
	lecturer=$(awk '$0 == "[LECTURER]" {i=1;next};i && i++ <= 1' "$1")
	courseID=$(awk '$0 == "[COURSEID-SECTION]" {i=1;next};i && i++ <= 1' "$1")
	machineNAME=$(awk '$0 == "[MACHINE]" {i=1;next};i && i++ <= 1' "$1")
	classinfo=$classtime', '$day

	email_list=$lecturer', '$admin
	echo "Sending Notification"
	python /home/paol/paol-code/scripts/notification/sendemail.py "$machineNAME" "$courseID" "$classinfo" "${email_list//,}"
}


scheduleScriptRun() {

	m=$1
	h=$2
	wd=$3

	if [[ $((10#$m - 9)) -lt 0 ]]; then
		m=$((10#$m + 51)) && h=$((10#$h - 1))
		# echo "$h:$m"
	else
		m=$((10#$m - 9)) 
	fi

	crontab -l > mycron

	if $(! crontab -l | grep -q '/home/paol/paol-code/scripts/notification/machineCheck.sh schedule'); then
		echo '00 5 * * 1-5 /home/paol/paol-code/scripts/notification/machineCheck.sh schedule' >> mycron
		echo '00 23 * * 1-5 /home/paol/paol-code/scripts/notification/cronFix.sh' >> mycron
	fi

	echo "$m $h * * $wd /home/paol/paol-code/scripts/notification/machineCheck.sh notify" >> mycron
	crontab mycron
	rm mycron

}


calcTimeDifference() {
	hrs=$1
	min=$2
	currentH=$3
	currentM=$4

	scheduledTime=$((10#$hrs * 3600 + 10#$min * 60))
	# echo "scheduledTime $scheduledTime"
	currentTime=$((10#$currentH * 3600 + 10#$currentM * 60))
	# echo "currentTime $currentTime"

	difference=$((10#$scheduledTime - 10#$currentTime))
	# echo $difference

	if [[ difference -lt 600 && difference -ge 0 ]]; then
		# echo "1"
		return 1
	else
		return 0
		# echo "0"
	fi
			
}	


checkTime() {
	for time in ${1//,};
	do
		for hour in ${time%-*''}; 
		do
			for minute in ${hour#''*:};
			do
				# echo "hour2 ${hour%:*''}"
				# echo "minute $minute"

				if [ "$script_mode" == "notify" ]; then
					calcTimeDifference "${hour%:*''}" "$minute" "$currentHour" "$currentMinute"
					local res=$?
					# echo $res

					if [ "$res" == "1" ]; then
						notificationInfo "$info_file"
						echo "Notification Sent"
					fi
				else
					echo "Scheduling Machine-Check/Notification"
					dowNumber=$(date --date=$weekday +'%w')
					scheduleScriptRun "$minute" "${hour%:*''}" "$dowNumber" 
					# echo "No Notification Needed"
				fi
			done
		done
	done
}


main() {

	lectureday=$(awk '$0 == "[CLASSTIME-WEEKDAY]" {i=1;next};i && i++ <= 1' "$info_file")
	# echo $lectureday
	classhours=$(awk '$0 == "[CLASSTIME-WEEKDAY]" {i=1;next};i && i++ <= 1' "$info_file")
	# echo $classhours

	datenow=$(date '+%H %M %A')

	currentHour=$(echo "$datenow" | awk '{print $1}')
	currentMinute=$(echo "$datenow" | awk '{print $2}')
	weekday=$(echo "$datenow" | awk '{print $NF}')

	for info in ${lectureday//,}; 
	do
		classtime=${info%/*''}
		day=${info#''*/}

		if [ "$weekday" == "$day" ]; then
			checkTime "$classtime"
		fi
	done

}

main

exit 0
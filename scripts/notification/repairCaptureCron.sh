#!/bin/bash

addToCron() {

	crontab -l > mycron
	echo "$1 $2 $3 $4 * $5 $6 $7 $8" >> mycron
	crontab mycron
	rm mycron

}

main() {

	scheduledCrons=$(crontab -l)

	path="/home/paol/paol-code/build-PAOL-LecCap-*/PAOL-LecCap-GUI"
	pathStandard="/home/paol/paol-code/build-PAOL-LecCap-GUI-Desktop_Qt_5_3_GCC_64bit-Debug/PAOL-LecCap-GUI"

	datenow=$(date '+%H %M %d %m')
	currentHour=$(echo "$datenow" | awk '{print $1}')
	currentMinute=$(echo "$datenow" | awk '{print $2}')
	currentDay=$(echo "$datenow" | awk '{print $3}')
	currentMonth=$(echo "$datenow" | awk '{print $4}')

	while IFS= read -r line; do

		checkjob=$(echo "$line" | awk '{print $6}')

		if [[ "$checkjob" == "$path" || "$checkjob" == "$pathStandard" ]]; then
			echo "capture cronjob: $line"
			minute=$(echo "$line" | awk '{print $1}')
			hour=$(echo "$line" | awk '{print $2}')
			day=$(echo "$line" | awk '{print $3}')
			mon=$(echo "$line" | awk '{print $4}')
			term=$(echo "$line" | awk '{print $(NF-2)}')
			course=$(echo "$line" | awk '{print $(NF-1)}')
			duration=$(echo "$line" | awk '{print $NF}')
			# echo "hr: $hora"
			# echo "min: $minuto"
			# echo "dia: $dia"
			# echo "mes: $mes"
			# echo "duracao: $duration"
			# echo "term: $term"
			# echo "course: $course"
			scheduledTime=$((10#$hour * 3600 + 10#$minute * 60))
			# echo "scheduledTime $scheduledTime"
			currentTime=$((10#$currentHour * 3600 + 10#$currentMinute * 60))
			# echo "currentTime $currentTime"

			difference=$((10#$currentTime - 10#$scheduledTime))
			# echo "difference $difference"

			# if [[ difference -lt 600 && difference -ge 0 ]]; then

			if [[ "$currentDay" == "$day" && "$currentMonth" == "$mon" ]]; then

				if [[ difference -lt $((10#$duration)) && difference -ge 0 ]]; then

					timeleft=$(( 10#$duration - 10#$difference - 60))
					# echo "timeleft: $timeleft"

					if [[ $((10#$currentMinute + 2)) -eq 60 ]]; then
						currentMinute=$((10#$currentMinute = 01 )) && currentHour=$((10#$currentHour + 1))
					else
						currentMinute=$((10#$currentMinute + 2)) 
					fi

					addToCron "$currentMinute" "$currentHour" "$currentDay" "$currentMonth" "$path" "$term" "$course" "$timeleft"

					echo "Added new job to the crontab."

				else
					echo "No repair-cronjob needs to be added for this capture."

				fi
			else
				echo "Capture cronjob not scheduled for current day."
			fi
		fi

	done <<< "$scheduledCrons"
}

main

exit 0

# 59 13 29 11 * /home/paol/paol-code/build-PAOL-LecCap-GUI-Desktop_Qt_5_3_GCC_64bit-Debug/PAOL-LecCap-GUI F17 Comp171-3 3270  ## Sample cronjob

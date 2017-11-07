#!/bin/bash
rmt_runcheck=/home/paol/Desktop/workspace/Up-Running-Check/notification
rmt_runcheck=/home/paol/paol-code/scripts/notification/machines

if [ "$#" -ne 1 ]; then
	echo "Usage: ./rr.sh <script_mode(schedule or notify>"
	exit
fi

script_mode=$1


if [[ "$script_mode" == "schedule" ]]; then
	for directory in $rmt_runcheck/*;
	do
		machine=$(basename "$directory")
		for f2 in $directory/*;
		do
			if [[ $f2 == *.txt ]]; then
				echo "Verifying if $machine needs Machine-Check SCHEDULING for the day"
				/home/paol/paol-code/scripts/notification/scheduleNotification.sh "$f2" "$script_mode"
			fi
		done
	done
else
	if [[ "$script_mode" == "notify" ]]; then
		for directory in $rmt_runcheck/*;
		do
			machine=$(basename "$directory")
			ping -c 3 "$machine" > /dev/null 2>&1
			if [ $? -ne 0 ]; then
				echo "MACHINE $machine is DOWN"
				for f2 in $directory/*;
				do
					if [[ $f2 == *.txt ]]; then
						echo "Verifying if $machine needs notification."
						/home/paol/paol-code/scripts/notification/scheduleNotification.sh "$f2" "$script_mode"
					fi
				done
			else
				echo "MACHINE $machine is UP"
			fi 
		done
	fi
fi

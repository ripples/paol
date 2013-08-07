#!/bin/bash

schedFile="/home/paol/paol-code/cron_temp.txt"
calendar=$(hostname)

scriptDir=$(pwd)
thisScript=$(basename $0)
updateCalJob="#0 * * * * $scriptDir/$thisScript"

mvn -q exec:java -Dexec.mainClass="edu.umass.cs.ripples.paol.CalendarParser" -Dexec.args=$calendar
status=$?
if [ $status = 0 ]; then
	(cat $schedFile; echo "$updateCalJob") | crontab -
	echo "Updated jobs in crontab"
else
	echo "Problem in calendar parser. Check for errors"
fi

# prior code for reference/future usage
# cat $schedFile | while read job; do
	# #find any jobs that match the one we want to add
	# jobmatch=$(crontab -l | grep "${job//[*]/[*]}")
	# #add job if no matches were found
	# if [ -z "$jobmatch" ]; then
		# (crontab -l; echo "$job") | crontab -
	# fi
# done
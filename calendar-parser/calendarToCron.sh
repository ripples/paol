#!/bin/bash

schedFile="/home/paol/paol-code/cron_temp.txt"
calendar=$(hostname)
log="/home/paol/calendarToCronOut.log"

parserDir="/home/paol/paol-code/calendar-parser"
thisScript="$parserDir/calendarToCron.sh"
updateCalJob="0 * * * * $thisScript"
processAllJob="15 1 * * * /home/paol/paol-code/scripts/process/processAll.sh"

date=$(date)
echo "================================$date================================" >> $log
cd $parserDir
mvn -q exec:java -Dexec.mainClass="edu.umass.cs.ripples.paol.CalendarParser" -Dexec.args=$calendar >> $log
status=$?
if [ $status = 0 ]; then
	(cat $schedFile; echo "$updateCalJob"; echo "$processAllJob") | crontab -
	echo "Updated jobs in crontab" >> $log
else
	echo "Problem in calendar parser. Check for errors" >> $log
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
#!/bin/bash

schedFile="/home/paol/paol-code/cron_temp.txt"
# By default, set calendar name to name of this machine
# If an input is given to this script, set the calendar name to the input
calendar=$(hostname)
if [ $1 ]; then
	calendar=$1
fi
log="/home/paol/calendarToCronOut.log"

parserDir="/home/paol/paol-code/calendar-parser"
thisScript="$parserDir/calendarToCron.sh"
updateCalJob="0 * * * * $thisScript $calendar"
processAllJob="15 1 * * * /home/paol/paol-code/scripts/process/processAll.sh"

# Get the location of the capture program
captureProgram=$(ls /home/paol/paol-code/build*/PAOL-LecCap-GUI)
# Construct the arguments for the parser
parserArgs="$captureProgram $calendar"

date=$(date)
echo "================================$date================================" >> $log
cd $parserDir
mvn -q exec:java -Dexec.mainClass="edu.umass.cs.ripples.paol.CalendarParser" -Dexec.args="$parserArgs" >> $log
status=$?
if [ $status = 0 ]; then
	(cat $schedFile; echo "$updateCalJob"; echo "$processAllJob") | crontab -
	echo "Updated jobs in crontab" >> $log
else
	echo "Problem in calendar parser. Check for errors" >> $log
fi

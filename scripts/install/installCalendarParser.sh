#!/bin/bash

cd /home/paol/paol-code/calendar-parser/
mvn compile
name=$HOSTNAME
echo $name
mvn -q exec:java -Dexec.mainClass="edu.umass.cs.ripples.paol.CalendarParser" -Dexec.args="$name"

## Calendar Parser
This is the code that interacts with Google Calendars to generate cron jobs, initialize calendars, etc. Currently, calendar initialization is not being used.

### How Calendar Parser reads the calendar
Calendar Parser takes in the name of the calendar you want to read from. The calendar name should not have spaces. Within the given calendar, each event occurrence represents one lecture. The name of an event is considered the course title, and this is used to determine where the files for the lecture should be stored. A typical course title would be something like "CMPSCI220: Programming Methodology". There are a number of cases to consider when naming events:

* Spaces are converted to underscores. For example, if the event name is "CMPSCI 220", the lecture will be saved in a file under the directory "CMPSCI_220".
* Only the part of the course title before the colon is considered. For example, the event name "CMPSCI220: Programming to Methodology" maps to the directory "CMPSCI220". If the event name does not contain a colon, it maps to the directory with the same name (replacing spaces with underscores).

### Logging in
The first time CalendarParser is run (either by itself or as part of calendarToCron.sh), the program will open a page in a browser, where you can enter your Google account information. Then you must allow PAOL to read your calendar by clicking "Accept" on the following page. Your credentials are stored in ~/.credentials/calendar.json. To use a different account, delete ~/.credentials/calendar.json and run CalendarParser again.

### What running CalendarParser.java actually does
CalendarParser.java is responsible for generating cron_temp.txt, which stores the cron lines that should be added by calendarToCron.sh. When the Java code is run, it reads from the given calendar the events for the next three days and writes cron lines that will run fullCapture.sh during the span of the events, plus five minutes before and after the event. For example, if a lecture lasts from 1:00-2:00, the corresponding cron job will run from 12:55 to 2:05. CalendarParser.java does not actually add these lines to crontab; calendarToCron.sh is responsible for that.

### How to compile and run CalendarParser.java by itself
To compile, run
```
mvn compile
```
To execute, run the following line:
```
mvn -q exec:java -Dexec.mainClass="edu.umass.cs.ripples.paol.CalendarParser" -Dexec.args="<calendarName>"
```
where <calendarName> is replaced by the name of your calendar. The calendar name should not have spaces.

### What running calendarToCron.sh actually does
calendarToCron.sh clears the current crontab, then adds the following jobs to the crontab:
* The jobs for capturing lectures. It does this by calling CalendarParser.java to generate cron_temp.txt, then adding the lines in the text file to the crontab.
* The job to run processAll.sh, which processes all lectures in the raw directory, then uploads them to the server. This job runs every night at 1:15am.
* The job to run calendarToCron.sh every hour at XX:00. Adding this job causes cron to check the calendar every hour for updates.

### How to run calendarToCron.sh
calendarToCron can optionally take an argument specifying the name of the calendar to read from. If this argument is not given, it reads the calendar with the same name as the machine name. Below are two sample calls to calendarToCron.sh:

```Shell
# Reads the calendar MachineName if the name of the machine is MachineName
./calendarToCron.sh

# Reads the calendar myCalendar
./calendarToCron.sh myCalendar
```

To stop the machine from automatically checking the calendar, simply delete the calendarToCron.sh job from the crontab.

### pom.xml
pom.xml describes the project and lists dependencies. Maven reads from this file and downloads any necessary plugins and resources. Compilation information is also specified here.

### Installing Maven and JDK
This project requires Maven 2.2.1 or higher and Java 1.5 or higher with JDK. To install them, call the following commands:
```
sudo apt-get install default-jre
sudo apt-get install default-jdk
sudo apt-get install maven2
```


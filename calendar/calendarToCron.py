from __future__ import print_function
from googleapiclient.discovery import build
from httplib2 import Http
from oauth2client import file, client, tools
from crontab import CronTab
import datetime
import socket
import dateutil.parser

'''
Script to populate the crontab with default events and recording events from the calendar.
Make sure client_secret is in the working directory.
If the hostname is not found in the calendar names, it set to paolCap303 by default
Make sure dateutil, oauth2client, python-crontab, google-api-python-client, packages are installed.
'''


# initializes the google calendar api - provides credentials
def iniService():
    SCOPES = 'https://www.googleapis.com/auth/calendar.readonly'
    store = file.Storage('credentials.json')
    creds = store.get()
    if not creds or creds.invalid:
        flow = client.flow_from_clientsecrets('client_secret.json', SCOPES)
        creds = tools.run_flow(flow, store)
    service = build('calendar', 'v3', http=creds.authorize(Http()))
    return service


# gets list of calendars
def getCalID(service):
    # dictionary to hold calendar IDs - calName:calID
    calIDs = dict()

    # iterates through calendar entries and gets names and IDs
    page_token = None
    while True:
        calendar_list = service.calendarList().list(pageToken=page_token).execute()
        for entry in calendar_list['items']:
            # print(entry['summary'] + ' ' + entry['id'])
            calIDs[entry['summary']] = entry['id']
        page_token = calendar_list.get('nextPageToken')
        if not page_token:
            break
    return calIDs


# gets most current semester date from semesterDates.txt file
def getSemesterDate():
    # path for current term and file object
    curTermPath = '/home/paol/paol-code/semesterDates.txt'
    curTermFile = open(curTermPath, 'r')

    # gets current term from semesterDates.txt
    lines = curTermFile.readlines()
    termLine = lines[len(lines)-1]
    term = termLine.split(":")[0]
    curTermFile.close()

    return term


# gets calendar events for the day
def getCalEvents(calIds, service):
    # stores event strings
    formatEvents = []

    # is the calendar name found in the dictionary
    found = False

    # name to be found is host
    calName = socket.gethostname()

    # iterates through dictionary for calendar name
    for item in calIds:
        if calName in item:
            found = True
            break

    # name is defaulted to paolCap303 if host name is not present in calendars
    if found:
        print('Calendar found')
    else:
        print('Calendar not found, defaulting to paolCap303')
        calName = 'paolCap303'

    # for item in calIds:
    #     print(item + ' ' + calIds[item])

    # start and end time for the day - range provided for daily events
    startTime = datetime.datetime.today().replace(hour=0, minute=0, second=0, microsecond=0).isoformat() + 'Z'
    endTime = datetime.datetime.today().replace(hour=23, minute=55, second=0, microsecond=0).isoformat() + 'Z'

    # get list of events from calendar
    events_result = service.events().list(calendarId=calIds[calName], timeMin=startTime,
                                          timeMax=endTime, singleEvents=True,
                                          orderBy='startTime').execute()

    # get info from event list
    events = events_result.get('items', [])

    if not events:
        print('No upcoming events found')

    # populates events array with formatted info
    i = 0
    for event in events:
        # event start and end
        start = event['start'].get('dateTime', event['start'].get('date'))
        end = event['end'].get('dateTime', event['end'].get('date'))

        # use dateutil to parse back to datetime object
        sParsed = dateutil.parser.parse(start)
        eParsed = dateutil.parser.parse(end)

        # time delta to figure out duration
        delta = eParsed - sParsed
        duration = delta.seconds + 120

        # add items to array
        formatEvents.append(sParsed.strftime('%H:%M') + ' ' + eParsed.strftime('%H:%M') + ' ' + event['summary'] + ' ' + str(duration))
        i = i + 1
        # print(sParsed.strftime('%H:%M'), eParsed.strftime('%H:%M'), event['summary'], duration)
    return formatEvents


# creates default cronjobs
def defaultCron(my_cron):
    my_cron.remove_all()

    # Job to create recordings directory if machine is new
    jobOne = my_cron.new(command="mkdir /home/paol/recordings/ -p", comment="default")
    jobOne.hour.on(1)
    jobOne.minute.on(15)	

    # Job to restart machine in order to release cameras
    jobSix = my_cron.new(command="sudo reboot", comment="default")
    jobSix.hour.on(0)
    jobSix.minute.on(01)
	
    # Job to mark date in cron log
    jobFive = my_cron.new(command="date >> /home/paol/recordings/cron.log", comment="default")
    jobFive.hour.on(1)
    jobFive.minute.on(30)

    # Job to update cron jobs from google calendar (runs 1 hour after midnight every day)
    jobTwo = my_cron.new(command="/home/paol/paol-code/calendar/updateCron.sh >> /home/paol/recordings/cron.log", comment="default")
    jobTwo.hour.on(1)
    jobTwo.minute.on(0)

    # Job for email notifications (runs at 5pm)
    jobThree = my_cron.new(command="/home/paol/paol-code/scripts/notification/emailNotification.sh >> /home/paol/recordings/cron.log", comment="default")
    jobThree.hour.on(17)
    jobThree.minute.on(0)

    # Job to upload all recordings for the day (runs at 11pm)
    jobFour = my_cron.new(command="/home/paol/paol-code/scripts/upload/uploadAll.sh >> /home/paol/recordings/cron.log", comment="default")
    jobFour.hour.on(23)
    jobFour.minute.on(0)

    my_cron.write()


# updates and creates recording jobs
def updateCrontab(events, my_cron):
    # go through events and process strings for cron
    for event in events:
        # split time string
        timeSplit = event.split(' ')
        hmTime = timeSplit[0].split(":")

        # gets minute for job, sets it to a minute before
        minute = hmTime[1]

        # gets hour for job
        hour = hmTime[0]

        # convert and adjust time for recording
        intMin = int(minute)
        intHour = int(hour)

        if intMin == 0:
            intMin = 59
            intHour = intHour - 1
        else:
            intMin = intMin - 1

        minute = str(intMin)
        hour = str(intHour)

        # get class name
        className = timeSplit[2]

        # get job duration
        duration = timeSplit[3]
        semesterDate = getSemesterDate()

        # Cron Job to be sent to crontab
        job = my_cron.new(
            command="/home/paol/paol-code/build-PAOL-LecCap-GUI-Desktop_Qt*/PAOL-LecCap-GUI " +
                    semesterDate + ' ' + className + ' ' + duration + ' >> /home/paol/recordings/cron.log', comment="job")
        job.hour.on(hour)
        job.minute.on(minute)
        my_cron.write()


def main():
    # crontab for paol
    my_cron = CronTab(user='paol')

    # api service object
    service = iniService()

    # calendar names and IDs
    calIds = getCalID(service)

    # formatted events for today
    fEvents = getCalEvents(calIds, service)

    # create recording and default jobs
    defaultCron(my_cron)
    updateCrontab(fEvents, my_cron)


main()

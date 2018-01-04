# PAOL Machine Notification

## Notification Schema

The notification script runs on the presentation machine, and it checks all IPS in the [machines folder](https://github.com/ripples/paol/tree/master/scripts/notification/machines). Each IP corresponds to the machines the recording system runs on. For each "IP folder", there are .txt files that have the necessary information to **schedule notifications** and **notify (if necessary)** ([Sample Notification File](https://github.com/ripples/paol/blob/master/scripts/notification/Sample_NotificationFile.txt)), for every course that machine should record.

For now, all the information has to be with the sample standard formatting (**like this: 11:00-11:52/Monday, 11:00-11:52/Wednesday, 11:00-11:52/Friday**) and in one single line after the file section (**like:[LECTURER]**), so the script can read it correctly.

The script has two modes: one for scheduling the notification before the classes and one that notifies i.e, sends a notification email to the addresses found on each file for each course, if the machine is not on when it was supposed to be.

## Running Machine Notification

You should run it only on the presentation machine. Using the command line:
```
$ ./machineCheck.sh schedule
```

The notification scheduling runs automatically every morning to make sure the professor will be notified about the classes that day if the machine isn't up and running 5 minutes before its starting time.

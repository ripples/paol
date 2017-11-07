import sys
import smtplib
import datetime

machine = sys.argv[1]
courseID = sys.argv[2]
classtime = sys.argv[3]
email_list = sys.argv[4]
email_list = email_list.split()

now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
fromaddr = 'paolcalic@gmail.com'
toaddrs  = email_list
msg = 'Subject: Machine ' + machine + " isn't running\n\nThis machine must be off. Turn it on for the recording.\nCourse ID-Section: " + courseID + "\nClasstime: " + classtime + "\nDate: " + str(now)
username = 'paolcalic@gmail.com'
password = 'secretPAOL'
server = smtplib.SMTP('smtp.gmail.com:587')
server.ehlo()
server.starttls()
server.login(username,password)
server.sendmail(fromaddr, toaddrs, msg)
server.quit()

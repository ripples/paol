import sys
import smtplib
import datetime

machine = sys.argv[1]
courseID = sys.argv[2]
classtime = sys.argv[3]
errorMess = sys.argv[4]
errorMess = errorMess.replace("_", " ")

email_list = "mdesmery@ithaca.edu"
email_list = email_list.split(",")

TO = email_list
SUBJECT = 'ERROR'
TEXT = errorMess
now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
fromaddr = 'paolcalic@gmail.com'
toaddrs = email_list
msg = 'Subject: ' + machine + "\r\n\nError Message: " + errorMess + "\nCourse ID-Section: " + courseID + "\nClasstime: " + classtime + "\nDate: " + str(now)

# body = '\r\n'.join(['To %s' % toaddrs,
#                     'From: %s' % fromaddr,
#                     'Subject: %s' % SUBJECT,
#                     '', TEXT])

username = 'paolcalic@gmail.com'
password = 'secretPAOL'
server = smtplib.SMTP_SSL('smtp.gmail.com', 465)
server.ehlo()
# server.starttls()
server.login(username,password)
server.sendmail(fromaddr, email_list, msg)
server.quit()

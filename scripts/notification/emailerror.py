import sys
import smtplib
import datetime

courseID = sys.argv[1]
errorMess = sys.argv[2]
errorMess = errorMess.replace("_", " ")

email_list = "mdesmery@ithaca.edu,vmarkov@ithaca.edu,pdickson@ithaca.edu"
#email_list = "mdesmery@ithaca.edu"
email_list = email_list.split(",")

TO = email_list
SUBJECT = 'ERROR'
TEXT = errorMess
now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
fromaddr = 'paolcalic@gmail.com'
toaddrs = email_list
msg = 'Subject: ERROR NOTIFICATION'+ "\r\n\nError Message: \n" + errorMess + "\nDate: " + str(now)

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

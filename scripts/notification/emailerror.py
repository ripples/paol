import sys
import smtplib
import datetime
import socket

errorMess = sys.argv[1]
errorMess = errorMess.replace("_", " ")

email_list = "mdesmery@ithaca.edu,vmarkov@ithaca.edu"
email_list = email_list.split(",")

# construct message body
TO = email_list
SUBJECT = 'ERROR'
TEXT = errorMess
now = datetime.datetime.now().strftime("%Y-%m-%d %H:%M")
fromaddr = 'paolcalic@gmail.com'
toaddrs = email_list
msg = 'Subject: ERROR NOTIFICATION FROM: ' + socket.gethostname() + "\r\n\nError Message: \n" + errorMess + "\nDate: " + str(now)

# provide credentials
username = 'paolcalic@gmail.com'
password = 'secretPAOL'

# contact server and send message
server = smtplib.SMTP_SSL('smtp.gmail.com', 465)
#server.set_debuglevel(1)
server.ehlo()
# server.starttls()
server.login(username,password)
server.sendmail(fromaddr, email_list, msg)
server.quit()

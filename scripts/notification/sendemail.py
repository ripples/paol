import sys
import smtplib
import datetime

now = datetime.datetime.now()
fromaddr = 'paolcalic@gmail.com'
toaddrs  = 'paolcalic@gmail.com'
msg = 'Subject:' + sys.argv[1] + " isn't running\n\nThis machine must be off\nDate: " + str(now)
username = 'paolcalic@gmail.com'
password = 'secretPAOL'
server = smtplib.SMTP('smtp.gmail.com:587')
server.ehlo()
server.starttls()
server.login(username,password)
server.sendmail(fromaddr, toaddrs, msg)
server.quit()

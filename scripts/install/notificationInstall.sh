#!/bin/bash

#Script to setup the email notifications (standard configuration) automatically

#Install msmtp and ca-certificates

sudo apt-get install msmtp ca-certificates

#Clear /etc/msmtprc content
sudo bash -c "> /etc/msmtprc"

#Add the standard email configuration code
sudo bash -c "cat >> /etc/msmtprc <<EOF
defaults
tls on
tls_starttls on
tls_trust_file /etc/ssl/certs/ca-certificates.crt
 
account default
host smtp.gmail.com
port 587
auth on
user paolcalic@gmail.com
password secretPAOL
from paolcalic@gmail.com
#logfile /var/log/msmtp.log
EOF"

# Make sure the text editor is reable by anyone

sudo chmod 0644 /etc/msmtprc


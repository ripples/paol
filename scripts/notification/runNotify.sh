#!/bin/bash

echo "Generating notify information..."
python /home/paol/paol-code/scripts/notification/generateNotifyInfo.py
sleep 3
echo "Files generated"
echo "Sending email..."
python /home/paol/paol-code/scripts/notification/parseErrors.py
echo "Email sent"
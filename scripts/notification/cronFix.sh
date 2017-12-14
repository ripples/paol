#!/bin/bash
crontab -r
crontab -l > mycron

if $(! crontab -l | grep -q '@reboot /home/paol/paol-code/scripts/notification/machineCheck.sh schedule'); then
		echo '@reboot /home/paol/paol-code/scripts/notification/machineCheck.sh schedule' >> mycron
		echo '@reboot /home/paol/paol-code/scripts/notification/repairCaptureCron.sh' >> mycron
		echo '00 5 * * 1-5 /home/paol/paol-code/scripts/notification/machineCheck.sh schedule' >> mycron
		echo '00 23 * * 1-5 /home/paol/paol-code/scripts/notification/cronFix.sh' >> mycron
	fi

crontab mycron
rm mycron
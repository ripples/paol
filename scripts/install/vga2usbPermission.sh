#!/bin/bash

#script to setup vga2usb permissions so that plugging in device will 
# automatically change permissions on it

cd 
cd paol/paol-code/scripts/install/vga2usbPermissions/
sudo chmod 001 chmodBash
sudo cp 85-paol_rule.rules /etc/udev/rules.d/
sudo /etc/init.d/udev restart

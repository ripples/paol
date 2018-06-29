#!/bin/bash

# install vga2usb driver, dkms is needed as dependency 
# setup vga2usb permissions so that plugging in device will 
# automatically change permissions on it

cd
cd Downloads
sudo apt-get install dkms
wget http://updates.epiphan.com/downloads/Drivers/Ubuntu/vga2usb-3.30.6.5-ubuntu-4.15.0-22-generic-x86_64-41987-1799.deb
sudo dpkg -i vga2usb-*
cd 
cd paol-code/scripts/install/vga2usbPermissions/
sudo chmod 001 chmodBash
sudo cp 85-paol_rule.rules /etc/udev/rules.d/
sudo /etc/init.d/udev restart

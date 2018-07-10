#!/bin/bash

sudo apt-get update -y
sudo apt-get upgrade -y

#basic programs paul can't live without
sudo apt-get install -y emacs
sudo apt-get install -y gi
sudo apt-get install -y gnome-session-flashback
sudo apt-get install -y imagemagick

#install google calendar api
sudo apt-get install -y default-jre
sudo apt-get install -y default-jdk
sudo apt-get install -y ant
sudo apt-get install -y maven
sudo apt-get install -y maven2

#from http://code.google.com/p/qt-opencv-multithreaded/wiki/Documentation
/home/paol/paol-code/scripts/install/opencvInstall.sh 

#install ffmpeg
/home/paol/paol-code/scripts/install/ffmpegInstall.sh 

#install gstreamer
/home/paol/paol-code/scripts/install/gstreamerInstall.sh 

#install notifications
/home/paol/paol-code/scripts/install/notificationInstall.sh

#install imagemagick
sudo apt-get install imagemagick

#install rule to change permissions on vga2usb
/home/paol/paol-code/scripts/install/vga2usbPermission.sh

#install python modules for calendar updater
/home/paol/paol-code/scripts/install/cronCalendar.sh

#!/bin/bash

sudo apt-get update -y
sudo apt-get upgrade -y

#basic programs paul can't live without
sudo apt-get install -y emacs
sudo apt-get install -y gi
sudo apt-get install -y gnome-session-fallback

#install google calendar api
sudo apt-get install -y openjdk-7-jdk
sudo apt-get install -y ant

#from http://code.google.com/p/qt-opencv-multithreaded/wiki/Documentation
/home/paol/paol-code/scripts/install/opencvInstall.sh &

#install ffmpeg
/home/paol/paol-code/scripts/install/ffmpegInstall.sh &

#install rule to change permissions on vga2usb
/home/paol/paol-code/scripts/install/vga2usbPermission.sh

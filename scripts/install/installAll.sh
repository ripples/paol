#!/bin/bash

sudo apt-get update
sudo apt-get upgrade

#basic programs paul can't live without
sudo apt-get install -y emacs
sudo apt-get install -y gi
sudo apt-get install -y gnome-session-fallback

#install google calendar api
sudo apt-get install -y openjdk-7-jdk
sudo apt-get install -y ant

#from http://code.google.com/p/qt-opencv-multithreaded/wiki/Documentation
/home/paol/scripts/install/opencvInstall.sh

#install ffmpeg
/home/paol/scripts/install/ffmpegInstall.sh

#install rule to change permissions on vga2usb
/home/paol/scripts/install/vga2usbPermission.sh

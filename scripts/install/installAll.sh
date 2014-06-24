#!/bin/bash

sudo apt-get update
sudo apt-get upgrade

#basic programs paul can't live without
sudo apt-get install -y emacs
sudo apt-get install -y git
sudo apt-get install -y gnome-session-fallback

#install google calendar api
sudo apt-get install -y openjdk-7-jdk
sudo apt-get install -y ant

#from http://code.google.com/p/qt-opencv-multithreaded/wiki/Documentation
./opencvInstall.sh

#install ffmpeg
./ffmpegInstall.sh

#install rule to change permissions on vga2usb
./vga2usbPermission.sh

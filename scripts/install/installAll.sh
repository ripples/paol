#!/bin/bash

sudo apt-get update
sudo apt-get upgrade

#basic programs paul can't live without
sudo apt-get install emacs
sudo apt-get install git
sudo apt-get install gnome-session-fallback

#install google calendar api
sudo apt-get install openjdk-7-jdk
sudo apt-get install ant

#from http://code.google.com/p/qt-opencv-multithreaded/wiki/Documentation
./opencvInstall.sh

#install ffmpeg
./ffmpegInstall.sh

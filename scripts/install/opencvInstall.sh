#!/bin/bash

#additional packages needed by openCV
sudo apt-get install build-essential cmake libv4l-dev pkg-config
sudo apt-get libgtk2.0-dev
sudo apt-get libtiff4-dev
sudo apt-get libjasper-dev
sudo apt-get libavformat-dev
sudo apt-get libswscale-dev
sudo apt-get libavcodec-dev
sudo apt-get libjpeg-dev
sudo apt-get libpng-dev

#install openCV
cd Downloads
wget http://downloads.sourceforge.net/project/opencvlibrary/opencv-unix/2.4.9/opencv-2.4.9.zip
unzip opencv-2.4.9.zip
cd opencv-2.4.9/
mkdir release 
cd release
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
sudo ldconfig

#needed by qtCreator
sudo apt-get install libgl1-mesa-dev

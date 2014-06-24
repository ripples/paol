#!/bin/bash

#additional packages needed by openCV
sudo apt-get install -y build-essential cmake libv4l-dev pkg-config
sudo apt-get install -y libgtk2.0-dev
sudo apt-get install -y libtiff4-dev
sudo apt-get install -y libjasper-dev
sudo apt-get install -y libavformat-dev
sudo apt-get install -y libswscale-dev
sudo apt-get install -y libavcodec-dev
sudo apt-get install -y libjpeg-dev
sudo apt-get install -y libpng-dev

#install openCV
cd
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
sudo apt-get install -y libgl1-mesa-dev

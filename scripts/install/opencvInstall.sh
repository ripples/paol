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
wget https://sourceforge.net/projects/opencvlibrary/files/opencv-unix/3.4.1/opencv-3.4.1.zip
unzip opencv-3.4.1
cd opencv-3.4.1/
mkdir release 
cd release
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local ..
make
sudo make install
sudo ldconfig

#needed by qtCreator
sudo apt-get install -y libgl1-mesa-dev

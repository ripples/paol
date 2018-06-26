#!/bin/bash

#Script to install gstreamer and v4l packages. Necessary for video record.

sudo apt-get update

#Install v4l-utils package

sudo apt-get install v4l-utils

#Install gstreamer package and plugins

sudo apt-get install libgstreamer1.0-0
sudo apt-get install gstreamer1.0-plugins-base
sudo apt-get install gstreamer1.0-plugins-good
sudo apt-get install gstreamer1.0-plugins-bad
sudo apt-get install gstreamer1.0-plugins-ugly
sudo apt-get install gstreamer1.0-libav
sudo apt-get install gstreamer1.0-doc
sudo apt-get install gstreamer1.0-tools
sudo apt-get install gstreamer-tools
sudo apt-get install ubuntu-restricted-extras



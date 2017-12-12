#!/bin/bash

#Script to install gstreamer and v4l packages. Necessary for video record.

sudo apt-get update

#Install v4l-utils package

sudo apt-get install v4l-utils

#Install gstreamer package and plugins

sudo apt-get install gstreamer1.0*
sudo apt-get install gstreamer-tools
sudo apt-get install ubuntu-restricted-extras


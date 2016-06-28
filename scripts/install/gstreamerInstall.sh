#!/bin/bash

#Script to install gstreamer and v4l packages. Necessary for video record.

sudo apt-get update

#Install v4l-utils package

sudo apt-get install v4l-utils

#Install gstreamer package and plugins

sudo apt-get install gstreamer1.0*
sudo apt-get install gstreamer-tools

#Configure C920 Cameras into H264 at specific resolution and framerate

v4l2-ctl --set-fmt-video=width=800,height=448,pixelformat=1
v4l2-ctl --set-parm=24

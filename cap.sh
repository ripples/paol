#!/bin/sh

##
# Simple gstreamer pipeline to record a video captured from a webcam (through v4l) to OGG file while
# showing the video also on the screen.
#
# Sample usage:
#
# Record to webcam.ogg:
#
#   ./camrec.sh
#
# Record to output.ogg:
#
#   ./camrec.sh output.ogg
#
# To adjust capture width/hight or set the V4L device see variables below.
#
# Pipeline:
#
# Capture video from V4L source, convert to ffmpegcolorspace, split result (with tee and two queues)
# to two separate path. One shows currently captured video on screen (xvimagesink), the other one 
# encodes the captured video to OGG-container with theoraenc and writes the result to a file (filesink). 
#
#                                                                      +-------------+
#                                                                   +--| xvimagesink |
#                                                                 [Q]  +-------------+
# +---------+  +-----------------+  +------------------+  +-----+ /
# | v4l2src |--| video/x-raw-yuv |--| ffmpegcolorspace |--| tee |
# +---------+  +-----------------+  +------------------+  +-----+ \
#                                                                 [Q]  +-----------+  +--------+  +----------+
#                                                                   +--| theoraenc |--| oggmux |--| filesink |
#                                                                      +-----------+  +--------+  +----------+
##

##
# --- Settings
##

# Output file 
OUTPUT=$1
VIDEONUM=$2
AUDIOSTR=$3

# capture device to use
V4LDEV="/dev/video${VIDEONUM}"

# capture width
WIDTH=640

# capture height
HEIGHT=480

# bitrate for theoraenc
BITRATE=256

##
# --- END of settings
##


if [ "X$1" = "X" ]
then
   OUTPUT="webcam"
fi

echo "**"
echo "* Starting recording from webcam@${V4LDEV} to: ${OUTPUT}@${WIDTH}x${HEIGHT}"
echo "* Using OGG-container and THEORA-encoding with bitrate: ${BITRATE}" 
echo "* Recording Audio from ${AUDIOSTR}" 
echo "* Press CTRL+C to end recording"
echo "**"

#
# -e switch makes gst to close the OGG container properly when exited with CTRL+C
#
gst-launch-1.0 -e \
   v4l2src device=${V4LDEV} ! video/x-raw,width=${WIDTH},height=${HEIGHT} ! \
   videoconvert ! \
   tee name="splitter" ! \
   queue ! videorate ! video/x-raw ! \
theoraenc bitrate=${BITRATE} ! queue ! oggmux name=mux pulsesrc device=${AUDIOSTR} ! \
audio/x-raw,rate=32000,channels=2,depth=16 ! audioconvert ! queue ! \
vorbisenc ! mux. mux. ! queue ! filesink location=${OUTPUT}.ogv

ffmpeg -i ${OUTPUT}.ogv -vcodec libx264 ${OUTPUT}.mp4

#gst-launch-1.0 -e v4l2src device=/dev/video1 ! video/x-h264,width=320, height=240, framerate=24/1 ! tee name=myvid myvid. ! queue ! mux.video_0 pulsesrc device="alsa_input.usb-046d_Logitech_Webcam_C930e_B15BB16E-02.analog-stereo" ! audio/x-raw,rate=32000,channels=2,depth=16 ! audioconvert ! voaacenc ! aacparse ! queue ! mux.audio_0 mp4mux name=mux ! filesink location=./videoLarge.mp4
#gst-launch-1.0 -e v4l2src device=/dev/video1 ! video/x-h264,width=320, height=240, framerate=24/1 ! mp4mux name=mux ! filesink location=./videoLarge.mp4


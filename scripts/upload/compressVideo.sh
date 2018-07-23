#!/bin/bash

# Set the first argument as the lecture directory to upload
outDir=$1
#exit 0
cd $outDir
#ffmpeg -i lecture.mp4 -acodec libfdk_aac -b:a 128k -vcodec libx264 -preset ultrafast -b:v 260k -profile:v baseline -level 3.0 -pix_fmt yuv420p -flags +aic+mv4 -threads 0 -r 24 video.mp4
ffmpeg -r 24 -i lecture.mp4 -c:a aac -b:a 128k -vcodec libx264 -preset ultrafast -b:v 260k -profile:v baseline -level 3.0 -pix_fmt yuv420p -flags +aic+mv4 -threads 0 -r 24 video.mp4

rm -f lecture.mp4


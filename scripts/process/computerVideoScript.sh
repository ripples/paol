#what to add if computer has videos
if [ "$(ls -A $outDir/computerVideoFrames)" ]; then
    screenDir="$outDir/computerVideoFrames/" 
    screenFirst="$(ls $screenDir/ForCompMovie_000001*)"
    screenFirst="$(basename $screenFirst)"
    echo "Will make screen movie with: $screenDir $screenFirst"
    cd $outDir/computerVideoFrames/
    ffmpeg -r 15 -i ForCompMovie_%06d.png -an -pass 1 -vcodec libx264 -vpre slowfirstpass -b 200k -threads 8 -f rawvideo -y /dev/null 
    ffmpeg -r 15 -i ForCompMovie_%06d.png -an -pass 2 -vcodec libx264 -vpre normal -b 200k -threads 8 -y $outDir/computerVideo/video.mp4
    ffmpeg -r 15 -i ForCompMovie_%06d.png -an -vcodec libtheora -threads 8 -y $outDir/computerVideo/video.ogv
    #ffmpeg -r 15 -i ForCompMovie_%06d.png $outDir/computerVideo/uncompressed.avi
    cp $outDir/computerVideoFrames/screenVideo.txt $outDir/computerVideo/screenVideo.txt
    rm -rf $outDir/computerVideoFrames/
else
    echo "No screen video to make"
fi


To run the capture process 
1) Make sure that a cameraSetup.txt file is in the paol-code directory
2) Call fullCapture.sh with the following arguments SEMESTER COURSE DURATION (sec)
  example: fullCapture.sh Summer13 comp171 3600 

(note: videoCapture script is called by fullCapture.sh)

Whiteboard and VGA image  naming scheme:
The first six digits are image number for the given device, ie for camera one it would be 000000 for the first 000001 for the second, etc. the second number that is 10 digits long is the time (usual time from epoch). The final number is the device number. For our 4 camera sit-ups we would have 0-3 for this.

Example: cameraIn000000-1371053717-0.png

There will be an image 000000 for each device.

# paol
The code/scripts contained in this directory are all that is required in order
 to run PAOL and upload the processed lectures to the web server. 
## Note When Doing Hardware Configuration
It appears that we can only capture from one video device per rate synced bus. 
 The command lsusb will return the bus structure of the machine. From experience
 it is possible to put as many video devices on a non-rate synced bus as there  
 are ports but not on rate synced buses. Hanging ethernet hubs off of the
 non-rate synced bus does not give any additional working ports on that bus.

## Directory Structure of Input
(* lines need to be updated as we figure out what we want)

```
recordings
|-raw (unprocessed lectures
| |-Semester (ex: Fall13)
| | |-Course (ex: comp171)
| | | |-Class (ex: 06-12-2013--12-15-15)
| | | | |          MM-DD-YYYY--HH-MM-SS
| | | | |-video.mpeg
| | | | |-info
| | | | |-dataCam.log (log files are the output parts of capture script)
| | | | |-main.log
| | | | |-vidCam.log
| | | | |-computer
| | | | | |-vgaTwoUsbIn000000-1371053717-0.png
| | | | | |-...        (image#-time-vga2usb#)
| | | | |-wBoard
| | | | | |-cameraIn000000-1371053717-0.png
| | | | | |-...        (image#-time-camera#)
|-processed (same as raw)
|-readyToUpload
| |-Semester (ex: Fall13)
| | |-Course (ex: comp171)
| | | |-Class (ex: 06-12-2013--12-15-15)
| | | | |          MM-DD-YYYY--HH-MM-SS
*| | | | |-video.mpeg
| | | | |-info 
| | | | |-logs
| | | | | |-dataCam.log 
| | | | | |-main.log
| | | | | |-vidCam.log
| | | | | |-procComp.log
| | | | | |-procCam.log
| | | | |-computer
| | | | | |-computer1371053717-0.png
| | | | | |-...     (time-vga2usb#)
| | | | |-wBoard
| | | | | |-camera1371053717-0.png
| | | | | |-...  (time-camera#)
|-uploaded (same as readyToUpload)
```

The capture scripts generate all folders and files shown from class on down

## To Run

### cameraSetup.txt
This file must be created to run paol. It specifies where the video, whiteboard, and computer cameras are. Below is a sample cameraSetup.txt file:

```
0 1 Video
1 0 VGA2USB
2 0 Whiteboard
0 Audio
```

For the Video, VGA2USB, and Whiteboard lines, the first number corresponds to the input number. For the above example, PAOL would use /dev/video0 as the video camera, /dev/video1 as the VGA capture camera, and /dev/video2 as the whiteboard capture camera. The second number is a 0 or 1, depending on whether the image should be flipped. In the above example, the video camera input should be flipped.

For the Audio line, the number corresponds to which webcam's microphone to use as the audio for the video capture. In the above example, the video would use hw:0 as input.

### uploadConfig.txt
This file must be created to upload lectures. It specifies the local (capture) and remote (server) machines. Below is an example:

```
user: webmanic
host: present.cs.umass.edu
rmt_upload: /data1/www/media/uploaded
```

The example uploadConfig file tells the upload script to rsync to present.cs.umass.edu as user webmanic, and to put the lecture files in /data1/www/media/uploaded on present.cs.umass.edu.

### semesterDates.txt
This file is used by CalendarParser to assign semesters to lectures. This file is technically optional, but without it, no meaningful semester information is associated with the lectures. Below is an example semesterDates.txt file:
```
Su13: 6/1/13 8/31/13
F13: 9/3/13 12/6/13
W13: 12/16/13 1/18/14
S14: 1/21/14 4/30/14
```
The semester name comes before the colon. After the colon and a space should be first day of the semester, inclusive. After the first date, there should be a space and then the last day of the semester, inclusive. Dates with numbers that start with 0 are not guaranteed to be read properly by CalendarParser.

### How to capture and process lectures
```
% ~/paol-code/scripts/capture/fullCapture.sh Summer13 comp171 3600
Process:
% ~/paol-code/scripts/process/processAll.sh
Upload:
(Run as part of the process script)
```

### Compile the project via command line 
# prerequisites
sudo apt install build-essential

## run build.sh on paol-code directory. Make sure the path is correct

### Newer implementation python-paol

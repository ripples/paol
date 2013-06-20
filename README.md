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
This file must be created to run paol. It specifies where the video, whiteboard, and computer cameras are. Below is an example of the format:

```
0 Video
1 VGA2USB
2 Whiteboard
```
### Capture
```
% /paul-code/scripts/capture/fullCapture.sh Summer13 comp171 3600
Process:
% /paul-code/scripts/capture/processAll.sh
Upload:
(will be done as last step in process script, not yet implemented)
```

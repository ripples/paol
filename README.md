# paol
The code/scripts contained in this directory are all that is required in order
 to run PAOL and upload the processed lectures to the web server. 

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

### Capture
```
% /paul-code/scripts/capture/fullCapture.sh Summer13 comp171 3600
Process:
% /paul-code/scripts/capture/processAll.sh
Upload:
(will be done as last step in process script, not yet implemented)
```

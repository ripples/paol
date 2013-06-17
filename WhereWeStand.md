Capture works correctly if
1) The cameraSetup.txt file exists and is in the paol-code directory
2) There is a vga2usb signal when it is specified in the cameraSetup.txt file
   (we apparently have a solution to having to chmod the vga2usb when it is 
    plugged in)

Processing code will all be called when processing script is called but code 
does not function correctly. Currently whiteboard processing does processing
but doesn't actually do what we want and needs serious work to be fixed
(basically, new research to improve). Computer processing only actually copies 
images at this point, the processing code needs to be ported into the current
system from the threaded version of the paol code.

Uploading does not exist in any format.

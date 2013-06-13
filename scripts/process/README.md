Scripts to process the captured lectures.

To process all lectures:
-------------------------
./processAll.sh


processAll.sh
-------------------------
This script calls the processSingleLect.sh script until all lectures have been
processed.


processSingleLect.sh
--------------------------
This script processes the lecture it is given as an argument placing the output 
presentation into the readyToUpload directory and the raw data into the 
processed directory. Once this is completed it will (once upload script is 
written and added) upload the presentation to the web server.


processSingleLectFindLect.sh
--------------------------
This script looks into the recordings/raw directory and processes the first 
lecture it finds, placing the output presentation into the readyToUpload
directory and the raw data into the processed directory. Once this is completed
it will (once upload script is written and added) upload the presentation to
the web server.


ALSO:
---------------------------
computerVideoScript.sh
This is legacy script for if we ever get the computer processing that recognizes
videos working. This is the code to combine the still images into a video.

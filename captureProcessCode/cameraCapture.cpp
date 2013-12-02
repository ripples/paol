#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>

using namespace cv;

int main(int argc, char* argv[]) {
	//argv 1: directory to save images
	//argv 2: duration
	//argv 3: device type (VGA2USB or whiteboard)
	//argv 4: input label (/dev/video_)
	//argv 5: output device number
	char* saveDir = argv[1];
	int duration=atoi(argv[2]);
	char* devType = argv[3];
	int camLabel = atoi(argv[4]);
	int outDevNum = atoi(argv[5]);

	int frameCount=0;
	char filename[256];
	char* filenameStart;
	Mat frame;
	time_t startTime,cTime;
	int currentTime=0;
	int prevFrameTime=0;

	time(&startTime);

	//print all information related to device number and camera number for debugging
	printf("device: %s /dev/video%d number=%d\n",devType,camLabel,outDevNum);
	printf("Duration: %d\n",duration);

	//open connection to the device
	VideoCapture cam;
	try {
		cam = VideoCapture(camLabel);
	} catch (Exception& e) {
		printf(e.what());
		//return 1;
	}
	//if there is a failure while connecting to the device try again as long
	//as recording is still going on
	while ( currentTime<duration && !cam.isOpened()) {
	  cam = VideoCapture(camLabel);
	  time(&cTime);
	  currentTime=(int)(cTime-startTime);
	}
	
	if(strcmp("VGA2USB", devType) == 0)
	  filenameStart = "vgaTwoUsbIn";
	else {
	  filenameStart = "cameraIn";
	  cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	  cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	}
	
	cam.release();
	
	//capture data
	while ( currentTime<duration ) {
	  // if(strcmp("VGA2USB", devType) == 0) {
	    // // check if VGA state has been changed
	    // VideoCapture curVGA = VideoCapture(camLabel);
	    // if(curVGA.get(CV_CAP_PROP_FRAME_WIDTH) != cam.get(CV_CAP_PROP_FRAME_WIDTH)) {
	      // cam = curVGA;
	      // printf("VGA state has changed (plugged or unplugged)\n");
	    // }
	  // }
	  if(currentTime > prevFrameTime) {
	      prevFrameTime = currentTime;
		  printf("Current time: %d\n", currentTime);
		  cam = VideoCapture(camLabel);
		  if(strcmp("VGA2USB", devType) != 0) {
			cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
			cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
		  }
		  if(cam.get(CV_CAP_PROP_FRAME_WIDTH) != 0) {
			  cam >> frame;
			  //sprintf arguments
			  //saveDir: folder to write output to
			  sprintf(filename,"%s%s%06d-%10d-%1d.png",saveDir,filenameStart,frameCount,(int)cTime,outDevNum);
			  imwrite(filename,frame);
			  printf("%s\n",filename);
			  frameCount++;
		  }
		  cam.release();
	  }
	  time(&cTime);
	  currentTime=(int)(cTime-startTime);
	  fflush(stdout);
	}
	return 0;
	
}
//what the number in cap refers to:
// 0: video0 on laptops this is internal camera
// 1: video1 first usb video device plugged in
//    should be vga2usb and into usb2.0 slot
//       if this fails make sure that permissions one the vga2usb are correct
//       sudo chmod 777 /dev/vga2usb*
// 2: video2 second
//    should be camera for video and usb3.0 slot
// 3: video3 third
//    should be camera for whiteboard and sub3.0 slot

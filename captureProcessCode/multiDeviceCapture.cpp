#include "opencv/cv.h" 
#include "opencv/highgui.h" 

 
#include <iostream>
#include <istream>
#include <sstream>
#include <fstream>

#include <cstdlib>
#include <ctime>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

int main(int argc, char* argv[]) {
  //argv 1: directory to save whiteboard images
  //argv 2: directory to save computer images
  //argv 3: duration
  //argv 4: cameraSetup.txt
  ////argv 3: device type (VGA2USB or whiteboard)
  ////argv 4: input label (/dev/video_)
  ////argv 5: output device number
  char* whitebDir = argv[1];
  char* compDir = argv[2];
  int duration=atoi(argv[3]);

  /*char* devType = argv[3];
	int camLabel = atoi(argv[4]);
	int outDevNum = atoi(argv[5]);
  */

  
  int frameCount=0;
  char filename[256];
  char* filenameStart;
  Mat frame;
  time_t startTime,cTime;
  int currentTime=0;
  int setupTime=0;
  int prevFrameTime=0;
  
  time(&startTime);

  int deviceCount=0;
  vector <VideoCapture> dev;
  vector <bool> vga;
  vector <int> inDevNum;
  vector <int> outDevNum;
  vector <bool> flip;
  int whiteboardCount=0;
  int VGAcount=0;

  vector <string> camType;
  printf("start\n");

  //this will have to be done from a loop through the cameraSetup.txt file
  ifstream infile(argv[4]);
  string line;
  string placeHolder;
  int devNum;
  int devFlipNum;
  bool devFlip;

  if(infile.is_open()){
    while(getline(infile, line)){
      stringstream splitter(line);

      placeHolder=line.substr(0,1);
      devNum=atoi(placeHolder.c_str());

      placeHolder=line.substr(2,3);
      devFlipNum=atoi(placeHolder.c_str());
      if (devFlipNum==0)
	devFlip=false;
      else
	devFlip=true;
      
      placeHolder=line.substr(4,line.size());
      if (placeHolder=="VGA2USB"){
	inDevNum.push_back(devNum);
	dev.push_back(devNum);
	vga.push_back(true);
	flip.push_back(devFlip);
	outDevNum.push_back(VGAcount);
	VGAcount++;
	deviceCount++;
      } else if (placeHolder=="Whiteboard"){
	inDevNum.push_back(devNum);
	dev.push_back(devNum);
	vga.push_back(false);
	flip.push_back(devFlip);
	outDevNum.push_back(whiteboardCount);
	whiteboardCount++;
	deviceCount++;
	}
    }
  }

  time(&cTime);
  currentTime=(int)(cTime-startTime);
  setupTime=currentTime+1000;

  for (int i=0; i<deviceCount; i++){
    while ( currentTime<setupTime && 
	    !dev[i].isOpened() &&
	    !vga[i]){
      dev[i] = VideoCapture(inDevNum[i]);
      time(&cTime);
      currentTime=(int)(cTime-startTime);
    }
    if(!vga[i]){
      dev[i].set(CV_CAP_PROP_FRAME_WIDTH, 1920);
      dev[i].set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    }

    printf("%d device ",inDevNum[i]);
    if(vga[i])
      printf("VGA2USB\n");
    else
      printf("whiteboard\n");
  }

  //get images
  while ( currentTime<duration ) {
    time(&cTime);
    currentTime=(int)(cTime-startTime);

    for (int i=0; i<deviceCount;i++){
      if(!dev[i].isOpened() && !vga[i])
	dev[i] = VideoCapture(inDevNum[i]);

      if(vga[i]){
	filenameStart = "vgaTwoUsbIn";
	if(dev[i].isOpened())
	  dev[i].release();
	dev[i] = VideoCapture(inDevNum[i]);
      } else {
	filenameStart = "cameraIn";
      }
	
      printf("Current time: %d\n", currentTime);
      if(dev[i].isOpened()){
	dev[i] >> frame;
	if (flip[i])
	  cv::flip(frame,frame, -1);
	
	//sprintf arguments
	//saveDir: folder to write output to
	if(vga[i])
	  sprintf(filename,"%s%s%06d-%10d-%1d.png",compDir,filenameStart,frameCount,(int)cTime,outDevNum[i]);
	else
	  sprintf(filename,"%s%s%06d-%10d-%1d.png",whitebDir,filenameStart,frameCount,(int)cTime,outDevNum[i]);
	
	imwrite(filename,frame);
	printf("%s\n",filename);
      }
    }
    frameCount++;
    
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

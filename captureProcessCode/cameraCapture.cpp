#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <vector>
 
using namespace cv;

int main(int argc, char* argv[]) {
  //argv 1: directory to save computer images
  //argv 2: directory to save whiteboard
  //argv 3: duration
  //argv 4: camera file
  int compCount=0;
  int camCount=0;
  char filename[256];
  Mat frame;
  time_t startTime,cTime;
  int currentTime=0;
  int duration=atoi(argv[3]);
  vector<VideoCapture> vga2usb;
  int numVGA2USB=0;
  vector<VideoCapture> camera;
  int numCamera=0;
  FILE *cameraFile;
  int num;
  int i;
  char dev[256];
  vector<int> vga2usbLabels;

  time(&startTime);
  printf("%d\n",duration);

  cameraFile=fopen(argv[4],"r");
  while (fscanf(cameraFile,"%d %s\n",&num,dev)>0){
    printf("%d %s\n",num,dev);
    if (strcmp("VGA2USB",dev)==0){
      vga2usb.push_back(VideoCapture(num));
      numVGA2USB++;
	  vga2usbLabels.push_back(num);
    }
    if (strcmp("Whiteboard",dev)==0){
      camera.push_back(VideoCapture(num));
      camera[numCamera].set(CV_CAP_PROP_FRAME_WIDTH, 1920);
      camera[numCamera].set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
      numCamera++;
    }
  }
  printf("numcamera=%d\n",numCamera);
  printf("VGA labels: ");
  for(vector<int>::iterator i = vga2usbLabels.begin(); i != vga2usbLabels.end(); i++)
	printf("%d ", *i);
  printf("\n");
  //capture data
  while ( currentTime<duration ) {
    //computer capture
    if (numVGA2USB!=0){
      for(i=0;i<numVGA2USB;i++){ 
		// check if VGA state has been changed
		VideoCapture curVGA = VideoCapture(vga2usbLabels[i]);
		if(curVGA.get(CV_CAP_PROP_FRAME_WIDTH) != vga2usb[i].get(CV_CAP_PROP_FRAME_WIDTH)) {
			vga2usb[i] = curVGA;
			printf("VGA state has changed (plugged or unplugged)\n");
		}
		vga2usb[i] >> frame;
		time(&cTime);
		currentTime=(int)(cTime-startTime);
		//sprintf arguments
		//argv[1] folder to write output to
		sprintf(filename,"%svgaTwoUsbIn%06d-%10d-%1d.png",argv[1],compCount,(int)cTime,i);
		imwrite(filename,frame);
		printf("%s\n",filename);
      }
      compCount++;
    }
    
    if(numCamera!=0){
      //whiteboard capture
      for(i=0;i<numCamera;i++){ 
		camera[i] >> frame;
		time(&cTime);
		currentTime=(int)(cTime-startTime);
		//sprintf arguments
		//argv[1] folder to write output to
		sprintf(filename,"%scameraIn%06d-%10d-%1d.png",argv[2],camCount,(int)cTime,i);
		imwrite(filename,frame);
		printf("%s\n",filename);
      }
      camCount++;
    }
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

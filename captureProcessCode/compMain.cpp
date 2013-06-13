#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>

//#include "paolMat.h"
#include "fileIO.h"
 
using namespace cv;

void ComputerProcess(fileIO *disk){
  Ptr<paolMat> img;
  Ptr<paolMat> cleanImg;
  float temp;
  img = disk->read();

  while(img->src.data != NULL){
    disk->write(img);
      
    img = disk->read();
  }
};

int main(int argc, char* argv[]) {
  //argv 1: path to class images to process
  //argv 2: first image to process
  //argv 3: path to the output directory to write images to
  int count=0;
  char filename[256];
  Mat frame;
  time_t startTime,cTime;
  int currentTime=0;
  fileIO *filestuff1,*filestuff2,*filestuff3;
  Ptr<paolMat> im;
  
  filestuff1=new fileIO(argv[1],argv[2],"computer",argv[3]);

  ComputerProcess(filestuff1);

  return 0;
}

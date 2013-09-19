#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>

//#include "paolMat.h"
#include "fileIO.h"

#define bottomMask 115
#define thresholdDiff .0002
#define repeat 3//number of consecutive stable images necessary to consider stable

 
using namespace cv;

void ComputerProcess(fileIO *disk){
  Ptr<paolMat> current;
  Ptr<paolMat> previous;
  double percentDifference;
  int countStable;
  int timeDif;

  previous = disk->read();
  if (previous != NULL){
    current = disk->read();
  }

  while(current->src.data != NULL){
    if(previous->src.rows==current->src.rows && previous->src.cols==current->src.cols){
      current->difference(previous, 100, 0, bottomMask); 
      //percentDifference is (all the differences)/(size of the image)
      percentDifference=(double)current->difs/(double)(current->src.rows*current->src.cols);
    } else {
      percentDifference=1;
    }
    printf("perDif=%f\n",percentDifference);
    //if percentDifference is greater than the threshold       
    if(percentDifference>=thresholdDiff){
      printf(" perDif=%f thres=%f\n",percentDifference,thresholdDiff);
      //then if the number of identical images is greater then or equal to 3
      if (countStable>=repeat){
	//save image
	previous->name = "slide";
	previous->time = timeDif;
	disk->write(previous);//send to write
      } 
      
      countStable=0;
      timeDif=current->time;
    } else {
      countStable++;
    }
    previous->copy(current);
    current = disk->read();
  }
  //save last image
  if(previous != NULL){
    previous->name = "slide";
    previous->time = timeDif;
    disk->write(previous);
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

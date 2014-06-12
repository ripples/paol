#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>

//#include "paolMat.h"
#include "fileIO.h"
 
using namespace cv;

void WhiteBoardProcess(fileIO *disk){
  bool everyImage=true;//true if you want to process every image
  //if statement that uses everyImage should be removed once we can
  //tell which ones to save
  paolMat *cam;
  paolMat *old;
  paolMat *background;
  paolMat *backgroundRefined;
  paolMat *oldBackgroundRefined;
  paolMat *rawEnhanced;
  float numDif,refinedNumDif,saveNumDif;
  int count=0;

  old=new paolMat();
  background=new paolMat();
  backgroundRefined=new paolMat();
  oldBackgroundRefined=new paolMat();
  rawEnhanced=new paolMat();

  int firstTime = disk->time;
  cam = disk->read();
  old->copy(cam);

  cam = disk->read();
  while(cam->src.data != NULL){
    // only process once a minute
    if(disk->time % 60 == firstTime % 60 || everyImage) {

      //compare picture to previous picture and store differences in old->maskMin
      numDif=old->differenceMin(cam,40,1);

      //if there is enough of a difference between the two images
      if(numDif>.03){
        //set up a new % that represents difference
        refinedNumDif=old->shrinkMaskMin();
        count=0;
      } else {
        refinedNumDif=0;
      }

      //if the images are really identical, count the number of consecultive nearly identical images
      if (numDif < .000001)
        count++;

      //if the differences are enough that we know where the lecturer is or the images have been identical
      //for two frames, and hence no lecturer present
      if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
        //copy the input image and process it to highlight the text
        rawEnhanced->copy(cam);
        rawEnhanced->averageWhiteboard(20);
        rawEnhanced->enhanceText();
        //rawEnhanced->displayImage(*ui->imDisplay8);

        /////////////////////////////////////////////////////////////
        //identify where motion is
	
        //extend the area of differences and sweep differences for more solid area
        old->extendMaskMinToEdges();
        old->sweepDownMin();
        //keep only the solid area and grow that region
        old->keepWhiteMaskMin();
        old->growMin(8);
        //draw a convex hull around area of differences
        old->findContoursMaskMin();
        //fill in area surrounded by convex hull
        old->sweepDownMin();
        old->keepWhiteMaskMin();
        ///////////////////////////////////////////////////////////////////////
	
        //process to identify text location
	
        //smooth image
        cam->blur(1);
        //find edge information and store total edge information in 0 (blue) color channel of mask
        cam->pDrift();
        //grow the area around where the edges are found (if edge in channel 0 grow in channel 2)
        cam->grow(15,3);
        ////////////////////////////////////////////////////////////////////////////////
	
        //process to update background image
	
        //copy movement information into rawEnhanced and then expand to full mask
        rawEnhanced->copyMaskMin(old);
        rawEnhanced->maskMinToMaskBinary();
	
        //update the background image with new information
        background->updateBack2(rawEnhanced,cam);
	
        //copy the background image to one for processing
        backgroundRefined->copy(background);
        //darken text and set whiteboard to white
        backgroundRefined->darkenText();
        //copy text location information into mask
        backgroundRefined->copyMask(background);
        //////////////////////////////////////////////////
	
        //figure out if saves need to be made

        //count the number of differences in the refined text area between refined images
        saveNumDif = oldBackgroundRefined->countDifsMask(backgroundRefined);
	if (saveNumDif>.004)
	  disk->write(oldBackgroundRefined);

        //copy last clean whiteboard image
        oldBackgroundRefined->copy(backgroundRefined);

      }
    }
    //grab next image
    cam = disk->read();
  }
};
  

int main(int argc, char* argv[]) {
  //argv 1: path to class images to process
  //argv 2: first image to process
  //argv 3: path to write files to
  fileIO *filestuff;

  filestuff=new fileIO(argv[1],argv[2],"whiteBoard",argv[3]);

  WhiteBoardProcess(filestuff);

  return 0;
}

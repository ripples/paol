#include "opencv/cv.h" 
#include "opencv/highgui.h" 
#include <stdio.h> 
#include <iostream>
#include <cstdlib>
#include <ctime>

//#include "paolMat.h"
#include "fileIO.h"
 
using namespace cv;

void locateSpeaker(fileIO *disk){
  Ptr<paolMat> img;
  Ptr<paolMat> cleanImg;
  Ptr<paolMat> bgImg;
  Ptr<paolMat> lastImg;
  
  lastImg = disk->read();
  
  if(lastImg != NULL){
    img = disk->read();
    cleanImg = new paolMat(img);
    
    while(img->src.data != NULL){
      cleanImg->copy(img);
      //img->differenceLect(lastImg,150,1);
      img->differenceLect(lastImg,150,1);
      img->name="differenceLect";
      //img->writeMask();
	  
      cleanImg->copyNoSrc(img);
      disk->write(cleanImg);
	  
      lastImg = img;
      std::cout<<"LocateSpeaker:: Frame :"<<img->count<<std::endl;
      img = disk->read();
    }
  }
};

void WhiteBoardProcess(fileIO *disk){
  Ptr<paolMat> img;
  Ptr<paolMat> cleanImg;
  float temp;
  img = disk->read();

  Ptr<paolMat> background;
  background = new paolMat(img);
  background->name = "background";
  background->src = Scalar(255,255,255);

  img->blur(1);
  img->name = "1-blurred";
  img->write();
  
  Ptr<paolMat> alt;
  alt = new paolMat(img);
  alt->copy(img);

  img->pDrift();
  img->name = "2-pDrift";
  printf("before write mask\n");
  img->writeMask();

  img->grow(30,3);
  img->name = "3-grow";
  img->writeMask();
  
  Ptr<paolMat> old;
  old = new paolMat(img);
  old->name = "old";
  cleanImg = new paolMat(img);

  while(img->src.data != NULL){
    cleanImg->copy(img);
    //img->differenceLect(lastImg,150,1);
    img->differenceLect(old,150,1);
    img->name="4-differenceLect";
    img->writeMask();
    
    cleanImg->copyNoSrc(img);
    //disk->write(cleanImg);
    //std::cout<<"LocateSpeaker:: Frame :"<<img->count<<std::endl;

    temp=((float)img->difs/(img->src.rows*img->src.cols));
    //std::cout<<"WhiteBoardProc:: frame "<<img->count<<" had "<<img->difs<<" differences "<<std::endl;
    std::cout<<"WhiteBoardProc:: frame "<<img->count<<" had "<<temp<<" differences "<<std::endl;

    if(temp > .00000001){//.1) { //this should be img->difs > 20000, but debugging yo
      alt->copy(img);
      alt->differenceLect(old,150,1);
      
      img->blur(1);
      img->name="1-blur";
      img->write();
      
      img->pDrift();
      img->name="2-pdrift";
      img->writeMask();
      
      img->grow(30,3);
      img->name="3-grow";
      img->writeMask();
      
      old->name = "old";
      old->writeMask();
      img->name = "img";
      img->writeMask();
      img->threshedDifference(old);
      img->name = "threshedDifference";
      img->writeMask();
      
      
      alt->decimateMask(254);
      alt->decimateMask(254);
      alt->name = "alt-double-decimate";
      //
      alt->writeMask();
      alt->sweepDown();
      alt->name = "alt-sweptDown";
      //
      alt->writeMask();
      
      
      background->copyNoSrc(img);
      background->updateBackground(alt,img);
      background->name = "background-updated";
      //
      background->writeMask();
      background->cleanBackground(img);
      background->name = "background-clean";
      //
      background->write();
      //
      //background->writeMask();
      old->copy(img);
      //background->mask = img->mask.clone();
      disk->write(background);
      
    }
    
    //std::cout<<"WhiteBoardProc: about to pop"<<std::endl;
    img = disk->read();
    //std::cout<<"WhiteBoardProc: popped"<<std::endl;
  }
};

void WhiteBoardProcessQuick(fileIO *disk){
  Ptr<paolMat> img;
  Ptr<paolMat> cleanImg;
  float temp;
  img = disk->read();

  Ptr<paolMat> background;
  background = new paolMat(img);
  background->name = "background";
  background->copy(img);
  //background->src = Scalar(255,255,255);

  img->blur(1);
  //img->name = "1-blurred";
  //img->write();
  
  img->pDrift();
  //img->name = "2-pDrift";
  //printf("before write mask\n");
  //img->writeMask();
  
  img->grow(30,3);
  //img->name = "3-grow";
  //img->writeMask();
  /*
  Ptr<paolMat> old;
  old = new paolMat(img);
  old->name = "old";
  //cleanImg = new paolMat(img);
  */
  background->copy(img);
  //background->name = "background-updated";
  //  background->writeMask();
  background->updateBackground();
  //background->name = "background-clean-first";
  //background->write();
  background->cleanBackground(img);
  //background->name = "background-clean";
  //
  //background->writeMask();
  //  background->write();
  //testing for clarity
  //background->invert();
  //background->name="background-clean-inverted";
  //background->write();
  //background->invert();
  background->name="background-clean-darkened";
  background->darken();
  //background->write();
  disk->write(background);
  

  
  while(img->src.data != NULL){
    background->copy(img);
    //background->src = Scalar(255,255,255);
    img->blur(1);
    //img->name = "1-blurred";
    //img->write();
  
    img->pDrift();
    //img->name = "2-pDrift";
    printf("before write mask\n");
    img->writeMask();
    
    img->grow(30,3);
    //img->name = "3-grow";
    //img->writeMask();
    /*
    old->blockDiff(img);
    old->name="blockDiff";
    old->writeMask();
    old->copy(img);
    */
    background->copy(img);
    //background->name = "background-updated";
    //  background->writeMask();
    background->updateBackground();
    //background->name = "background-clean-first";
    //background->write();
    background->cleanBackground(img);
    //background->name = "background-clean";
    //
    //background->writeMask();
    //  background->write();
    //testing for clarity
    //background->invert();
    //background->name="background-clean-inverted";
    //background->write();
    //background->invert();
    background->name="background-clean-darkened";
    background->darken();
    //background->write();
    disk->write(background);
/*
    cleanImg->copy(img);
    //img->differenceLect(lastImg,150,1);
    img->differenceLect(old,150,1);
    img->name="4-differenceLect";
    img->writeMask();
    
    cleanImg->copyNoSrc(img);
    //disk->write(cleanImg);
    //std::cout<<"LocateSpeaker:: Frame :"<<img->count<<std::endl;

    temp=((float)img->difs/(img->src.rows*img->src.cols));
    //std::cout<<"WhiteBoardProc:: frame "<<img->count<<" had "<<img->difs<<" differences "<<std::endl;
    std::cout<<"WhiteBoardProc:: frame "<<img->count<<" had "<<temp<<" differences "<<std::endl;

    if(temp > .00000001){//.1) { //this should be img->difs > 20000, but debugging yo
      alt->copy(img);
      alt->differenceLect(old,150,1);
      
      img->blur(1);
      img->name="1-blur";
      img->write();
      
      img->pDrift();
      img->name="2-pdrift";
      img->writeMask();
      
      img->grow(30,3);
      img->name="3-grow";
      img->writeMask();
      
      old->name = "old";
      old->writeMask();
      img->name = "img";
      img->writeMask();
      img->threshedDifference(old);
      img->name = "threshedDifference";
      img->writeMask();
      
      
      alt->decimateMask(254);
      alt->decimateMask(254);
      alt->name = "alt-double-decimate";
      //
      alt->writeMask();
      alt->sweepDown();
      alt->name = "alt-sweptDown";
      //
      alt->writeMask();
      
      
      background->copyNoSrc(img);
      background->updateBackground(alt,img);
      background->name = "background-updated";
      //
      background->writeMask();
      background->cleanBackground(img);
      background->name = "background-clean";
      //
      background->write();
      //
      //background->writeMask();
      old->copy(img);
      //background->mask = img->mask.clone();
      disk->write(background);
      
      }*/
    
    //std::cout<<"WhiteBoardProc: about to pop"<<std::endl;
    img = disk->read();
    //std::cout<<"WhiteBoardProc: popped"<<std::endl;
  }
};

void WhiteBoardFoot(fileIO *disk){
  Ptr<paolMat> stable;
  stable = new paolMat();
  stable = disk->read();

  Ptr<paolMat> current;
  current = new paolMat(stable);

  stable->maskGrowRed(20);
  while(current->src.data != NULL){   
    
    stable->countDiffsMasked(current);
    //std::cout<<"Foot:: stable:"<<current->count<<" had "<<stable->difs<<" difs"<<std::endl;
    if(stable->difs > 2000){
      stable->name = "slide";
      disk->write(stable);
      //stable->copy(current);
      stable->finalWBUpdate(current);
      stable->maskGrowRed(20);
      stable->name = "FinalWBUpdategrown";
      //stable->writeMask();
    } else {
      stable->differenceDarken(current);
    }
    current = disk->read();
  }
};

int main(int argc, char* argv[]) {
  //argv 1: path to class images to process
  //argv 2: first image to process
  //argv 3: path to write files to
  int count=0;
  char filename[256];
  Mat frame;
  time_t startTime,cTime;
  int currentTime=0;
  fileIO *filestuff,*filestuff2,*filestuff3;
  Ptr<paolMat> im;
  //printf("here %s\n",argv[1]);
  //  filestuff=new fileIO(argv[1],argv[2],"frame","frameProcess");
  filestuff2=new fileIO(argv[1],argv[2],"whiteBoard",argv[3]);
  //filestuff3=new fileIO(argv[1],argv[2],"frameProcess","frameFoot");

  //locateSpeaker(filestuff);
  WhiteBoardProcessQuick(filestuff2);
  //WhiteBoardFoot(filestuff3);
  
  /*  im=filestuff->read();
  while(im->src.data){
    //printf("after read\n");
    //filestuff->print();
    filestuff->write(im);
    im=filestuff->read();
    }*/

  return 0;
}

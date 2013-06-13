//Including C++ Libs
#include <iostream>
#include <iterator>
#include <queue>
#include <string>
//#include <algorithm>
#include <cstdio>

//Open CV
#include "opencv/cv.h"
#include "opencv/highgui.h"

//#include "paolMat.h"
#include "fileIO.h"

//#define _debug_

using namespace cv;

fileIO::fileIO(char* pathIn,char* firstIm,char* writeNameIn,char* pathOutInput){
  sprintf(path,"%s",pathIn);
  sprintf(writeName,"%s",writeNameIn);
  sprintf(pathOut,"%s",pathOutInput);

  sscanf(firstIm,"%[^0-9]%06d-%10d-%d.png",readName,&countRead,&time,&cameraNum);
  //printf("here2 %s %d %d %d\n",readName,countRead,time,cameraNum);
  countWrite=countRead;
  countRead--;
  temp=new paolMat();
};
 
void fileIO::print(){
  printf("readwrite\n path=%s\n pathOut=%s readName=%s\n writeName=%s\n",path,pathOut,readName,writeName);
  printf(" time=%d\n camera#=%d\n countRead=%d\n countWrite=%d\n",time,cameraNum,countRead,countWrite);
};

Ptr<paolMat> fileIO::read(){
  char fullNamePath[256];
  Ptr<paolMat> img=new paolMat();

  int count, seconds, lastLoaded, tempCount, tempSeconds;
  char name[256];
  char fullName[256];

  countRead++;
  sprintf(name,"%s%06d-%10d-%d.png",readName,countRead,time,cameraNum);
  printf("readname=%s\n",name);
  sprintf(fullNamePath,"%s%s",path,name);
  //printf("readName:%s \n fullPath:%s\n",readName,fullNamePath);
  img->read(std::string(fullNamePath),std::string(readName),countRead,time);
  if(!temp->src.data){
    lastLoaded=time;
    while((time-lastLoaded)<20 && !img->src.data){
      time++;
      sprintf(name,"%s%06d-%10d-%d.png",readName,countRead,time,cameraNum);
      sprintf(fullNamePath,"%s%s",path,name);
      //printf("readName:%s \n fullPath:%s\n",readName,fullNamePath);
      img->read(std::string(fullNamePath),std::string(readName),countRead,time);
    }
  }

  return img;
};

void fileIO::write(Ptr<paolMat> imOut){
  imOut->write2(std::string(pathOut),std::string(writeName),cameraNum);
};

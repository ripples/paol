// ; -*-C++-*-
#ifndef _fileIO_h_
#define _fileIO_h_

#include "paolMat.h"

using namespace cv;

class fileIO
{
 public:
  Ptr<paolMat> temp;
  int cameraNum;
  int time;
  int countRead;
  int countWrite;
  char readName[512];
  char writeName[512];
  char path[512];
  char pathOut[512];
  
  fileIO(char* pathIn,char* firstIm,char* writeNameIn,char* pathOut);
  void print();
  Ptr<paolMat> read();  
  void write(Ptr<paolMat> imOut);  

};


#endif

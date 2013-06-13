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
  char readName[256];
  char writeName[256];
  char path[256];
  char pathOut[256];
  
  fileIO(char* pathIn,char* firstIm,char* writeNameIn,char* pathOut);
  void print();
  Ptr<paolMat> read();  
  void write(Ptr<paolMat> imOut);  

};


#endif

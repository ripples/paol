#ifndef WHITEBOARDPROCESS_H
#define WHITEBOARDPROCESS_H

#include "paolProcess.h"
#include "whiteboardworker.h"

class WhiteboardProcess : public paolProcess
{
public:
    WhiteboardProcess(string deviceUSB, int camNumIn, int wbNum, bool camFlipped, string lecturePath,bool runningGUI);
};

#endif // WHITEBOARDPROCESS_H

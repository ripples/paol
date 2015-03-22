#ifndef WHITEBOARDPROCESS_H
#define WHITEBOARDPROCESS_H

#include "paolProcess.h"
#include "whiteboardworker.h"

class WhiteboardProcess : public paolProcess
{
public:
    WhiteboardProcess(int camNumIn, int wbNum, bool camFlipped, string lecturePath);
};

#endif // WHITEBOARDPROCESS_H

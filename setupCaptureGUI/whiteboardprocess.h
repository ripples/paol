#ifndef WHITEBOARDPROCESS_H
#define WHITEBOARDPROCESS_H

#include "paolProcess.h"

class WhiteboardProcess : public paolProcess
{
private:
    // Field to keep track of which whiteboard device is being used
    int whiteboardNum;

    // Fields for whiteboard processing
    Mat currentFrame;
    Mat oldFrame;
    Mat oldMarkerModel;
    Mat oldRefinedBackground; // What the whiteboard from the oldFrame looks like
    int stableWhiteboardCount;

protected:
    void workOnNextImage();
    bool takePicture();
    void processImage();

public:
    WhiteboardProcess(int camNumIn, int wbNum, bool camFlipped, string lecturePath);
};

#endif // WHITEBOARDPROCESS_H

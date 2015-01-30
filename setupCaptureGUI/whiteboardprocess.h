#ifndef WHITEBOARDPROCESS_H
#define WHITEBOARDPROCESS_H

#include <stdlib.h>
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

    // Corner coordinates of the whiteboard images that this thread processes
    WBCorners corners;

    // Set the corners from a file
    static WBCorners getCornersFromFile(int wbNum);

protected:
    void workOnNextImage();
    bool takePicture();
    void processImage();
    void saveImageWithTimestamp(const Mat& image);
    void printToLog(char* format, ...);

public:
    WhiteboardProcess(int camNumIn, int wbNum, bool camFlipped, string lecturePath);
};

#endif // WHITEBOARDPROCESS_H

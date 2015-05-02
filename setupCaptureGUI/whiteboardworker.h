#ifndef WHITEBOARDWORKER_H
#define WHITEBOARDWORKER_H

#include "worker.h"
#include "PAOLProcUtils.h"

class WhiteboardWorker : public Worker {

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
    bool takePicture();
    void processImage();
    void saveImageWithTimestamp(const Mat& image);
    void printToLog(char* format, ...);
    void saveLastImage();
public:
    WhiteboardWorker(int camNumIn, int wbNum, bool camFlipped, string lecPath);
};

#endif // WHITEBOARDWORKER_H

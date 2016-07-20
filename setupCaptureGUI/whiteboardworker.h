#ifndef WHITEBOARDWORKER_H
#define WHITEBOARDWORKER_H

#include "worker.h"
#include "PAOLProcUtils.h"

class WhiteboardWorker : public Worker {

private:
    // Field to keep track of which whiteboard device is being used
    int whiteboardNum;
    string USB;

    // Fields for whiteboard processing
    Mat currentFrame;
    Mat currentRectified;
    Mat oldRectified;
    Mat blured;
    Mat normalizedCrossCorrelation;
    Mat oldNormalizedCrossCorrelation;
    Mat stablePixels;
    Mat refinedCurrentFrame;
    Mat currentWhiteboard;
    Mat oldWhiteboard;
    Mat notWhiteboard;
    Mat notWhiteboardGrown;
    Mat notWhiteboardEroded;
    Mat oldNotWhiteboardEroded;
    Mat notWhiteboardDifference;
    Mat notWhiteboardDifference2;

    Mat lastSaved;

    bool changedBoard;
    int consecutiveStableCount;
    int currentDifference;


    Mat oldMarkerModel;
    Mat oldRefinedBackground; // What the whiteboard from the oldFrame looks like
    int stableWhiteboardCount;

    // Corner coordinates of the whiteboard images that this thread processes
    WBCorners corners;

    // Set the corners from a file
    static WBCorners getCornersFromFile(string deviceUSB);

protected:
    bool takePicture();
    void processImage();
    void processImageOld();
    void saveImageWithTimestamp(const Mat& image);
    void saveImageWithTimestamp(const Mat& image,string name);
    void printToLog(char* format, ...);
    void saveLastImage();
public:
    WhiteboardWorker(string deviceUSB, int camNumIn, int wbNum, bool camFlipped, string lecPath);
};

#endif // WHITEBOARDWORKER_H

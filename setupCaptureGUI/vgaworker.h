#ifndef VGAWORKER_H
#define VGAWORKER_H

#include "worker.h"
#include "PAOLProcUtils.h"

class VGAWorker : public Worker
{
private:
    // Constants for processing computer image differences
    static const float COMP_DIFF_THRESHOLD = .0002;
    static const int COMP_REPEAT_THRESHOLD = 3;

    // Field to keep track of which VGA device is being used
    int vgaNum;

    // Fields for computer processing
    Mat currentScreen;
    Mat oldScreen;
    Mat lastStableScreen;
    int stableScreenCount;

    time_t stableTime;

protected:
    bool takePicture();
    void processImage();
    void saveImageWithTimestamp(const Mat& image);
    void printToLog(char* format, ...);
    void saveLastImage();

public:
    VGAWorker(int camNumIn, int vgaNum, bool camFlipped, string lecturePath, bool guiRunning);
};

#endif // VGAWORKER_H

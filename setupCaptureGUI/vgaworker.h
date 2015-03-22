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
    Mat oldScreen;
    Mat lastStableScreen;
    int stableScreenCount;

protected:
    bool takePicture();
    void processImage();
    void saveImageWithTimestamp(const Mat& image);
    void printToLog(char* format, ...);

public:
    VGAWorker(int camNumIn, int vgaNum, bool camFlipped, string lecturePath);
};

#endif // VGAWORKER_H

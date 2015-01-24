#ifndef VGAPROCESS_H
#define VGAPROCESS_H

#include "paolProcess.h"

class VGAProcess : public paolProcess
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

protected:
    void workOnNextImage();
    bool takePicture();
    void processImage();
    void saveImageWithTimestamp(const Mat& image);
    void printToLog(char* format, ...);

public:
    VGAProcess(int camNumIn, int vgaNum, bool camFlipped, string lecturePath);
};

#endif // VGAPROCESS_H
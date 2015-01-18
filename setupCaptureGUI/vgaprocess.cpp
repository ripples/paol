#include "vgaprocess.h"

VGAProcess::VGAProcess(int camNumIn, int vgaNum, bool camFlipped, string lecturePath)
{
    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    flipCam = camFlipped;
    deviceNum = camNumIn;

    // Initialize counts for processing
    saveImageCount = 0;
    capturedImageCount = 0;

    // Initialize the current frame and screen processing images
    takePicture();
    oldScreen = currentScreen.clone();
    lastStableScreen = Mat::zeros(oldScreen.size(), oldScreen.type());

    // Make the computer directory
    string compFolderPath = lecturePath + "/computer";
    string mkdirCommand = "mkdir -p " + compFolderPath;
    system(mkdirCommand.data());

    // Set saveImagePathFormat
    sprintf(saveImagePathFormat, "%s/computer%%d-%d.png", compFolderPath.data(), vgaNum);
}

void VGAProcess::workOnNextImage() {
    takePicture();
    processImage();
}

bool VGAProcess::takePicture() {
    // Get frame
    for(int i = 0; i < 5; i++) {
        camera >> currentScreen;
    }

    // Update time associated with current frame
    time(&currentImageTime);
    // Flip the image horizontally and vertically if the camera is upside-down
    if(flipCam) {
        flip(currentScreen, currentScreen, -1);
    }
    // Update number of images captured
    capturedImageCount++;
    // Let listeners know that an image was captured
    emit capturedImage(currentScreen, this);

    return currentScreen.data;
}

void VGAProcess::processImage() {
    float percentDifference = PAOLProcUtils::getVGADifferences(oldScreen, currentScreen);

    //if percentDifference is greater than the threshold
    if(percentDifference>=COMP_DIFF_THRESHOLD){
        //then if the number of identical images is greater than or equal to 3
        if (stableScreenCount>=COMP_REPEAT_THRESHOLD){
            // Update the last seen stable screen
            lastStableScreen = oldScreen.clone();
            // Save the stable screen
            saveImageWithTimestamp(lastStableScreen);
        }
        stableScreenCount=0;
    } else {
        stableScreenCount++;
    }
    oldScreen = currentScreen.clone();
}

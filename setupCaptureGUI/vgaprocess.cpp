#include "vgaprocess.h"

VGAProcess::VGAProcess(int camNumIn, int compNum, bool camFlipped, string lecturePath)
{
    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    flipCam = camFlipped;
    deviceNum = camNumIn;

    // Initialize counts for processing
    saveImageCount = 0;
    capturedImageCount = 0;
    stableScreenCount = 0;
    vgaNum = compNum;

    // Make the computer directory
    string compFolderPath = lecturePath + "/computer";
    string mkdirCommand = "mkdir -p " + compFolderPath;
    system(mkdirCommand.data());

    // Set saveImagePathFormat
    sprintf(saveImagePathFormat, "%s/computer%%d-%d.png", compFolderPath.data(), vgaNum);

    // Print the association between this process and the output
    qDebug("VGA %d: %p", vgaNum, this);
}

void VGAProcess::workOnNextImage() {
    bool gotPicture = takePicture();
    if(gotPicture) {
        // Print image capture success
        qDebug("Took picture in thread %p at time %ld", this, currentImageTime);
        // Let listeners know that an image was captured
        emit capturedImage(currentScreen, this);
        // Increase captured image count
        capturedImageCount++;

        processImage();
    }
}

bool VGAProcess::takePicture() {
    // Try to open the camera
    camera.open(deviceNum);
    // Get the next screen if the image feed is available
    if(camera.isOpened()) {
        // Update old screen
        oldScreen = currentScreen.clone();
        // Save current screen
        camera >> currentScreen;
        // Update time associated with current frame
        time(&currentImageTime);
        // Flip the image horizontally and vertically if the camera is upside-down
        if(flipCam) {
            flip(currentScreen, currentScreen, -1);
        }
        // Capture was successful, so return true
        return true;
    }
    // We could not pull a frame from the VGA2USB, so return false
    return false;
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
}

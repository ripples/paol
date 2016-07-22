#include "vgaworker.h"

VGAWorker::VGAWorker(int camNumIn, int compNum, bool camFlipped, string lecPath, bool guiRunning)
{
    runningGUI=guiRunning;

    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    flipCam = camFlipped;
    deviceNum = camNumIn;
    vgaNum = compNum;

    // Set the lecture path
    lecturePath = lecPath;

    // Set the log file
    stringstream ss;
    ss << lecturePath << "/logs/vga" << vgaNum << ".log";
    logFile = fopen(ss.str().c_str(), "w");
    assert(logFile != NULL);

    // Initialize counts for processing
    saveImageCount = 0;
    capturedImageCount = 0;
    stableScreenCount = 0;

    //set time for initial computer image to be displayed
    time(&currentImageTime);
    stableTime=currentImageTime;

    // Print the association between this process and the output
    printToLog("VGA %d: %p\n", vgaNum, this);
}

bool VGAWorker::takePicture() {
    // Reset connection with VGA2USB. Needed to handle VGA disconnections and grab
    // complete VGA images
    if(camera.isOpened())
        camera.release();
    camera.open(deviceNum);
    // Get the next screen if the image feed is available
    if(camera.isOpened() && camera.get(CV_CAP_PROP_FRAME_WIDTH) != 0) {
        // Update old screen
        if(oldScreen.data)
            oldScreen.release();
        oldScreen = currentScreen.clone();
        // Save current screen
        camera >> currentScreen;
        // Flip the image horizontally and vertically if the camera is upside-down
        if(flipCam) {
            flip(currentScreen, currentScreen, -1);
        }
        // Let listeners know that an image was captured
        if(runningGUI)
            emit capturedImage(currentScreen);
        // Capture was successful, so return true
        return true;
    }
    // We could not pull a frame from the VGA2USB, so return false
    return false;
}

void VGAWorker::processImage() {
    // If this is the first time processing, initialize VGA processing fields and return
    // without further processing
    if(!oldScreen.data) {
        oldScreen = currentScreen.clone();
        if(lastStableScreen.data)
            lastStableScreen.release();
        lastStableScreen = Mat::zeros(currentScreen.size(), currentScreen.type());
        return;
    }

    // Get difference between last and current images
    float percentDifference = PAOLProcUtils::getVGADifferences(oldScreen, currentScreen);

    // If computer image has not changed significantly
    if(percentDifference < COMP_DIFF_THRESHOLD) {
        // Update stable screen count
        stableScreenCount++;
        //copy the current image to the last stable image location
        if(lastStableScreen.data)
            lastStableScreen.release();
        lastStableScreen = currentScreen.clone();
    }
    else {
        // If there has been a change and the number of images stable before the change
        // were enough, save the last stable image
        if(stableScreenCount >= COMP_REPEAT_THRESHOLD) {
            saveImageWithTimestamp(lastStableScreen);
            realImageIsStored = true;
        }

        stableScreenCount = 0;
        stableTime=currentImageTime;
    }
}

void VGAWorker::saveImageWithTimestamp(const Mat& image) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/computer/computer" << "-" << vgaNum << "-" << stableTime << ".png";
    imwrite(ss.str(), image);

    // Print image save success
    printToLog("Saved picture from worker %p at time %ld\n", this, currentImageTime);
    // Let listeners know that an image was processed
    emit savedImage(image);

    // Increment number of saved images
    saveImageCount++;
}

void VGAWorker::printToLog(char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(logFile, format, argptr);
    va_end(argptr);
}

void VGAWorker::saveLastImage() {
    //if(stableScreenCount>0)
        saveImageWithTimestamp(lastStableScreen);
}

#include "vgaworker.h"

VGAWorker::VGAWorker(int camNumIn, int compNum, bool camFlipped, string lecPath)
{
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

    // Print the association between this process and the output
    printToLog("VGA %d: %p\n", vgaNum, this);
}

bool VGAWorker::takePicture() {
    // Try to open the camera
    camera.open(deviceNum);
    // Get the next screen if the image feed is available
    if(camera.isOpened() && camera.get(CV_CAP_PROP_FRAME_WIDTH) != 0) {
        // Update old screen
        oldScreen = currentFrame.clone();
        // Save current screen
        camera >> currentFrame;
        // Update time associated with current frame
        time(&currentImageTime);
        // Flip the image horizontally and vertically if the camera is upside-down
        if(flipCam) {
            flip(currentFrame, currentFrame, -1);
        }
        // Release the camera so it can be used again
        camera.release();
        // Capture was successful, so return true
        return true;
    }
    // We could not pull a frame from the VGA2USB, so return false
    return false;
}

void VGAWorker::processImage() {
    float percentDifference = PAOLProcUtils::getVGADifferences(oldScreen, currentFrame);

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
//    saveImageWithTimestamp(currentFrame);
}

void VGAWorker::saveImageWithTimestamp(const Mat& image) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/computer/computer" << currentImageTime << "-" << vgaNum << ".png";
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

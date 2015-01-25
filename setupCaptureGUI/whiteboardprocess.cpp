#include "whiteboardprocess.h"

WhiteboardProcess::WhiteboardProcess(int camNumIn, int wbNum, bool camFlipped, string lecPath)
{
    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    flipCam = camFlipped;
    deviceNum = camNumIn;
    whiteboardNum = wbNum;

    // Set lecture path
    lecturePath = lecPath;

    // Set the log file
    stringstream ss;
    ss << lecturePath << "/logs/whiteboard" << whiteboardNum << ".log";
    logFile = fopen(ss.str().c_str(), "w");
    assert(logFile != NULL);

    // Initialize counts for processing
    stableWhiteboardCount = 0;
    saveImageCount = 0;
    capturedImageCount = 0;

    // Print the association between this process and the output
    printToLog("WB %d: %p\n", whiteboardNum, this);
}

void WhiteboardProcess::workOnNextImage() {
    bool gotPicture = takePicture();
    if(gotPicture) {
        // Print image capture success
        printToLog("Took picture in thread %p at time %ld\n", this, currentImageTime);
        // Let listeners know that an image was captured
        emit capturedImage(currentFrame, this);
        // Increase captured image count
        capturedImageCount++;

        processImage();
    }
}

bool WhiteboardProcess::takePicture() {
    // Set old frame
    oldFrame = currentFrame.clone();
    // Set current frame
    camera >> currentFrame;
    // Update time associated with current frame
    time(&currentImageTime);
    // Flip the image horizontally and vertically if the camera is upside-down
    if(flipCam) {
        flip(currentFrame, currentFrame, -1);
    }
    return currentFrame.data;
}

void WhiteboardProcess::processImage() {
    // If this is the first time processing, initialize WB processing fields and return
    // without further processing
    if(!oldFrame.data) {
        oldRefinedBackground = Mat::zeros(currentFrame.size(), currentFrame.type());
        oldMarkerModel = Mat::zeros(currentFrame.size(), currentFrame.type());
        return;
    }

    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    PAOLProcUtils::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    // If there is a large enough difference, reset the stable whiteboard image count and do further processing
    if(numDif > .01) {
        // Reset stable whiteboard image count
        stableWhiteboardCount = 0;
        // Find true differences (ie. difference pixels with enough differences surrounding them)
        float refinedNumDif;
        Mat filteredDiffs;
        PAOLProcUtils::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);

        // Find if there are enough true differences to update the current marker and whiteboard models
        // (ie. professor movement or lighting change detected)
        if(refinedNumDif > .01) {
            // Identify where the motion (ie. the professor) is
            Mat movement = PAOLProcUtils::expandDifferencesRegion(filteredDiffs);
            // Rescale movement info to full size
            Mat mvmtFullSize = PAOLProcUtils::enlarge(movement);

            // Get the marker model of the current frame
            Mat currentMarkerWithProf = PAOLProcUtils::findMarkerWithCC(currentFrame);
            // Use the movement information to erase the professor
            Mat currentMarkerModel = PAOLProcUtils::updateModel(
                        oldMarkerModel, currentMarkerWithProf, mvmtFullSize);

            // Find how much the current marker model differs from the stored one
            float markerDiffs = PAOLProcUtils::findMarkerModelDiffs(oldMarkerModel, currentMarkerModel);
            // Save and update the models if the marker content changed enough
            if(markerDiffs > .0055) {
                // Save the smooth marker version of the old background image
                Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
                saveImageWithTimestamp(oldRefinedBackgroundSmooth);
                // Update marker model
                oldMarkerModel = currentMarkerModel.clone();
                // Update enhanced version of background
                Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentFrame, currentMarkerModel);
                oldRefinedBackground = PAOLProcUtils::updateModel(
                            oldRefinedBackground, whiteWhiteboard, mvmtFullSize);
            }
        }
    }
    // Otherwise, check if the frames are basically identical (ie. stable)
    else if(numDif < .000001) {
        stableWhiteboardCount++;
        // If the image has been stable for exactly three frames, the lecturer is not present, so we
        // can update the marker and whiteboard models without movement information
        if(stableWhiteboardCount == 3) {
            // Save the smooth marker version of the old background image
            Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
            saveImageWithTimestamp(oldRefinedBackgroundSmooth);
            // Update marker model
            Mat currentMarkerModel = PAOLProcUtils::findMarkerWithCC(currentFrame);
            oldMarkerModel = currentMarkerModel.clone();
            // Update enhanced version of background
            Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentFrame, currentMarkerModel);
            oldRefinedBackground = whiteWhiteboard.clone();
        }
    }
}

void WhiteboardProcess::saveImageWithTimestamp(const Mat& image) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/whiteboard/whiteBoard" << currentImageTime << "-" << whiteboardNum << ".png";
    imwrite(ss.str(), image);

    // Print image save success
    printToLog("Saved picture in thread %p at time %ld\n", this, currentImageTime);
    // Let listeners know that an image was processed
    emit savedImage(image, this);

    // Increment number of saved images
    saveImageCount++;
}

void WhiteboardProcess::printToLog(char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(logFile, format, argptr);
    va_end(argptr);
}

#include "whiteboardprocess.h"

WhiteboardProcess::WhiteboardProcess(int camNumIn, int wbNum, bool camFlipped, string lecturePath)
{
    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    flipCam = camFlipped;
    deviceNum = camNumIn;

    // Initialize counts for processing
    stableWhiteboardCount = 0;
    saveImageCount = 0;
    capturedImageCount = 0;

    // Initialize the current frame and whiteboard processing models
    takePicture();
    oldFrame = currentFrame.clone();
    oldRefinedBackground = Mat::zeros(oldFrame.size(), oldFrame.type());
    oldMarkerModel = Mat::zeros(oldFrame.size(), oldFrame.type());

    // Make the whiteboard directory
    string wbFolderPath = lecturePath + "/whiteboard";
    string mkdirCommand = "mkdir -p " + wbFolderPath;
    system(mkdirCommand.data());

    // Set saveImagePathFormat
    sprintf(saveImagePathFormat, "%s/whiteBoard%%d-%d.png", wbFolderPath.data(), wbNum);
}

void WhiteboardProcess::workOnNextImage() {
    takePicture();
    processImage();
}

bool WhiteboardProcess::takePicture() {
    // Get frame
    for(int i = 0; i < 5; i++) {
        camera >> currentFrame;
    }

    // Update time associated with current frame
    time(&currentImageTime);
    // Flip the image horizontally and vertically if the camera is upside-down
    if(flipCam) {
        flip(currentFrame, currentFrame, -1);
    }
    // Update number of images captured
    capturedImageCount++;
    // Let listeners know that an image was captured
    emit capturedImage(currentFrame, this);

    return currentFrame.data;
}

// TODO: Streamline this code
void WhiteboardProcess::processImage() {
    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    WhiteboardProcessor::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    //if there is enough of a difference between the two images
    float refinedNumDif = 0;
    Mat filteredDiffs = Mat::zeros(currentFrame.size(), currentFrame.type());
    if(numDif>.03){
        // Find the true differences
        WhiteboardProcessor::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);
        stableWhiteboardCount=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        stableWhiteboardCount++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && stableWhiteboardCount==2)){
        // Find marker strokes and enhance the whiteboard (ie. make all non-marker white)
        Mat currentMarker = WhiteboardProcessor::findMarkerWithCC(currentFrame);
        Mat whiteWhiteboard = WhiteboardProcessor::whitenWhiteboard(currentFrame, currentMarker);
        Mat enhancedMarker = WhiteboardProcessor::smoothMarkerTransition(whiteWhiteboard);

        /////////////////////////////////////////////////////////////
        //identify where motion is
        Mat diffHulls = WhiteboardProcessor::expandDifferencesRegion(filteredDiffs);
        Mat diffHullsFullSize = WhiteboardProcessor::enlarge(diffHulls);

        ///////////////////////////////////////////////////////////////////////

        // Get what the whiteboard currently looks like
        Mat currentWhiteboardModel = WhiteboardProcessor::updateWhiteboardModel(oldRefinedBackground, enhancedMarker, diffHullsFullSize);
        // Get what the marker currently looks like
        Mat newMarkerModel = WhiteboardProcessor::updateWhiteboardModel(oldMarkerModel, currentMarker, diffHullsFullSize);
        //////////////////////////////////////////////////

        //figure out if saves need to be made

        // Get a percentage for how much the marker model changed
        float saveNumDif = WhiteboardProcessor::findMarkerModelDiffs(oldMarkerModel, newMarkerModel);
        if (saveNumDif>.004){
            saveImageWithTimestamp(oldRefinedBackground);
        }
        //copy last clean whiteboard image
        oldRefinedBackground = currentWhiteboardModel.clone();
        oldMarkerModel = newMarkerModel.clone();
    }
    oldFrame = currentFrame.clone();
}

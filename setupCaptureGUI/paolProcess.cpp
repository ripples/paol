#include "paolProcess.h"

paolProcess::paolProcess(int camNumIn, bool whiteboardIn, string pathIn) : QThread(){
    whiteboard=whiteboardIn;

    capturedImageCount = 0;

    // Set up camera and take the first picture
    camera = VideoCapture(camNumIn);
    callTakePicture();

    if(whiteboard) {
        oldFrame = currentFrame.clone();
        oldRefinedBackground = Mat::zeros(oldFrame.size(), oldFrame.type());
        oldMarkerModel = Mat::zeros(oldFrame.size(), oldFrame.type());
    }
    else {
        oldScreen = currentScreen.clone();
        lastStableScreen = Mat::zeros(oldScreen.size(), oldScreen.type());
    }

    stableWhiteboardCount=0;
    saveImageCount = 0;
    deviceNum = camNumIn;

    if(whiteboard) {
        // Make the whiteboard directory
        string wbFolderPath = pathIn + "/whiteboard";
        string mkdirCommand = "mkdir -p " + wbFolderPath;
        system(mkdirCommand.data());

        // Set saveImagePathFormat
        sprintf(saveImagePathFormat, "%s/whiteBoard%%d-%d.png", wbFolderPath.data(), deviceNum);
    }
    else {
        // Make the computer directory
        string compFolderPath = pathIn + "/computer";
        string mkdirCommand = "mkdir -p " + compFolderPath;
        system(mkdirCommand.data());

        // Set saveImagePathFormat
        sprintf(saveImagePathFormat, "%s/computer%%d-%d.png", compFolderPath.data(), deviceNum);
    }

    keepRunning = true;
}

paolProcess::~paolProcess(){
    qDebug() << "Camera " << deviceNum << " has taken " << capturedImageCount << " frames.";
    if(camera.isOpened()) {
        camera.release();
    }
}

void paolProcess::run(){
    while(1){
        // Stop thread if keepRunning is false
        keepRunningMutex.lock();
        if(!keepRunning) {
            keepRunningMutex.unlock();
            break;
        }
        keepRunningMutex.unlock();

        // keepRunning was true, so continue processing
        workOnNextImage();
    }
    qDebug("Successfully stopped thread %p", this);
}

void paolProcess::callTakePicture(){
    if(whiteboard) {
        for(int i = 0; i < 5; i++) {
            camera >> currentFrame;
        }
        // Update time associated with current frame
        time(&currentImageTime);
        emit capturedImage(currentFrame, this);
    }
    else {
        for(int i = 0; i < 5; i++) {
            camera >> currentScreen;
        }
        // Update time associated with current frame
        time(&currentImageTime);
        emit capturedImage(currentScreen, this);
    }
    capturedImageCount++;
}

void paolProcess::workOnNextImage(){
    callTakePicture();
    if (whiteboard){
        processWB();
    }
    else{
        processComp();
    }
}

void paolProcess::processWB(){
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
            // Get the path to save the image to, then save
            char destination[256];
            sprintf(destination, saveImagePathFormat, currentImageTime);
            imwrite(destination, oldRefinedBackground);

            // Let listeners know that an image was processed
            emit processedImage(oldRefinedBackground, this);

            // Increment number of saved images
            saveImageCount++;
        }
        //copy last clean whiteboard image
        oldRefinedBackground = currentWhiteboardModel.clone();
        oldMarkerModel = newMarkerModel.clone();
    }
    oldFrame = currentFrame.clone();
}

void paolProcess::processComp(){
    float percentDifference = WhiteboardProcessor::difference(oldScreen, currentScreen);

    //if percentDifference is greater than the threshold
    if(percentDifference>=COMP_DIFF_THRESHOLD){
        //then if the number of identical images is greater than or equal to 3
        if (stableScreenCount>=COMP_REPEAT_THRESHOLD){
            // Update the last seen stable screen
            lastStableScreen = oldScreen.clone();

            // Get the path to save the image to, then save
            char destination[256];
            sprintf(destination, saveImagePathFormat, currentImageTime);
            imwrite(destination, lastStableScreen);

            // Let listeners know that an image was processed
            emit processedImage(lastStableScreen, this);

            // Increment number of saved images
            saveImageCount++;
        }

        stableScreenCount=0;
    } else {
        stableScreenCount++;
    }
    oldScreen = currentScreen.clone();
}

void paolProcess::onQuitProcessing() {
    qDebug("Received signal to stop thread %p", this);
    keepRunningMutex.lock();
    keepRunning = false;
    keepRunningMutex.unlock();
}

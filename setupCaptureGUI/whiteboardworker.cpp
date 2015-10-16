#include "whiteboardworker.h"

WhiteboardWorker::WhiteboardWorker(int camNumIn, int wbNum, bool camFlipped, string lecPath)
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
    qDebug("%s",ss.str().c_str());
    logFile = fopen(ss.str().c_str(), "w");
    assert(logFile != NULL);

    // Initialize counts for processing
    stableWhiteboardCount = 0;
    saveImageCount = 0;
    capturedImageCount = 0;

    corners = getCornersFromFile(wbNum);

    // Print the association between this process and the output
    printToLog("WB %d: %p\n", whiteboardNum, this);
}

bool WhiteboardWorker::takePicture() {
    // Set old frame
    oldFrame = currentFrame.clone();
    // Set current frame (grab five times so we get actual current frame)
    for(int i = 0; i < 5; i++)
        camera >> currentFrame;
    // Flip the image horizontally and vertically if the camera is upside-down
    if(flipCam) {
        flip(currentFrame, currentFrame, -1);
    }
    if(currentFrame.data) {
        // Image capture succeeded
        // Let listeners know that an image was captured
        emit capturedImage(currentFrame);
        return true;
    }
    else {
        // Image capture failed
        return false;
    }
}

void WhiteboardWorker::processImage() {
    Mat currentRectified = PAOLProcUtils::rectifyImage(currentFrame, corners);

    // If this is the first time processing, initialize WB processing fields and return
    // without further processing
    if(!oldFrame.data) {
        oldRefinedBackground = Mat::zeros(currentRectified.size(), currentRectified.type());
        oldMarkerModel = Mat::zeros(currentRectified.size(), currentRectified.type());
        return;
    }

    Mat oldRectified = PAOLProcUtils::rectifyImage(oldFrame, corners);

    float numDif;
    Mat allDiffs;
    PAOLProcUtils::findAllDiffsMini(allDiffs, numDif, oldRectified, currentRectified, 40, 1);

    // If there is a large enough difference, reset the stable whiteboard image count and do further processing
    if(numDif > .03) {
        // Reset stable whiteboard image count
        stableWhiteboardCount = 0;
        // Find true differences (ie. difference pixels with enough differences surrounding them)
        float refinedNumDif;
        Mat filteredDiffs;
        PAOLProcUtils::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);

        // Find if there are enough true differences to update the current marker and whiteboard models
        // (ie. professor movement or lighting change detected)
        if(refinedNumDif > .04) {
            // Identify where the motion (ie. the professor) is
            Mat movement = PAOLProcUtils::expandDifferencesRegion(filteredDiffs);
            // Rescale movement info to full size
            Mat mvmtFullSize = PAOLProcUtils::enlarge(movement);

            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Keep marker candidates intersecting with marker locations
            Mat currentMarkerWithProf = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            // Use the movement information to erase the professor
            Mat currentMarkerModel = PAOLProcUtils::updateModel(
                        oldMarkerModel, currentMarkerWithProf, mvmtFullSize);


            // Find how much the current marker model differs from the stored one
            float markerDiffs = PAOLProcUtils::findMarkerStrokeDiffs(oldMarkerModel, currentMarkerModel);
            /* test code to check out where the differences are occuring visually
            Mat strokeCheck=PAOLProcUtils::findMarkerStrokeDiffs2(oldMarkerModel, currentMarkerModel);
            saveImageWithTimestamp(strokeCheck,"strokecheck");
            */
            printToLog("numDif: %f\n", numDif);
            printToLog("refinedNumDif: %f\n", refinedNumDif);
            printToLog("markerDiffs: %f\n", markerDiffs);
            // Save and update the models if the marker content changed enough
            if(markerDiffs > .002) {
                // Only save the image if a meaningful has been stored
                if(realImageIsStored) {
                    //Mat oldRefinedBackgroundRectified = PAOLProcUtils::rectifyImage(oldRefinedBackground,corners);
                    saveImageWithTimestamp(oldRefinedBackground);
                }
                // Update marker model
                oldMarkerModel = currentMarkerModel.clone();
                // Update enhanced version of background
                Mat enhancedImage = PAOLProcUtils::enhanceColor(currentRectified);
                Mat enhancedImageDarkSharp = PAOLProcUtils::darkenText(currentMarkerModel,enhancedImage);
                oldRefinedBackground = PAOLProcUtils::updateModel(
                            oldRefinedBackground, enhancedImageDarkSharp, mvmtFullSize);
                realImageIsStored = true;
            }
        }
    }
    // Otherwise, check if the frames are basically identical (ie. stable)
    else if(numDif < .000001) {
        stableWhiteboardCount++;
        // If the image has been stable for exactly three frames, the lecturer is not present, so we
        // can update the marker and whiteboard models without movement information
        if(stableWhiteboardCount == 3) {
            // Only save the image if a meaningful has been stored
            if(realImageIsStored) {
                //Mat oldRefinedBackgroundRectified = PAOLProcUtils::rectifyImage(oldRefinedBackground,corners);
                saveImageWithTimestamp(oldRefinedBackground);
            }

            // Update marker mode
            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Use the movement information to erase the professor
            Mat currentMarkerModel = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            // Update enhanced version of background
            Mat enhancedImage = PAOLProcUtils::enhanceColor(currentRectified);
            Mat enhancedImageDarkSharp = PAOLProcUtils::darkenText(currentMarkerModel,enhancedImage);
            oldRefinedBackground = enhancedImageDarkSharp.clone();
            realImageIsStored = true;
        }
    }

    /*Ryan's method
     *
    // Get rectified versions of old and current frames
    Mat oldRectified = oldFrame.clone();//PAOLProcUtils::rectifyImage(oldFrame, corners);
    Mat currentRectified = currentFrame.clone();//PAOLProcUtils::rectifyImage(currentFrame, corners);

    //compare picture to previous picture and store differences in allDiffs
    float numDif;
    Mat allDiffs;
    PAOLProcUtils::findAllDiffsMini(allDiffs, numDif, oldRectified, currentRectified, 40, 1);

    // If there is a large enough difference, reset the stable whiteboard image count and do further processing
    if(numDif > .03) {
        // Reset stable whiteboard image count
        stableWhiteboardCount = 0;
        // Find true differences (ie. difference pixels with enough differences surrounding them)
        float refinedNumDif;
        Mat filteredDiffs;
        PAOLProcUtils::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);

        // Find if there are enough true differences to update the current marker and whiteboard models
        // (ie. professor movement or lighting change detected)
        if(refinedNumDif > .04) {
            // Identify where the motion (ie. the professor) is
            Mat movement = PAOLProcUtils::expandDifferencesRegion(filteredDiffs);
            // Rescale movement info to full size
            Mat mvmtFullSize = PAOLProcUtils::enlarge(movement);

            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Keep marker candidates intersecting with marker locations
            Mat currentMarkerWithProf = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            // Use the movement information to erase the professor
            Mat currentMarkerModel = PAOLProcUtils::updateModel(
                        oldMarkerModel, currentMarkerWithProf, mvmtFullSize);
            Mat currentMarkerModelGrown = PAOLProcUtils::grow(currentMarkerModel,3);
            // Find how much the current marker model differs from the stored one
            float markerDiffs = PAOLProcUtils::findMarkerModelDiffs(oldMarkerModel, currentMarkerModel);
            printToLog("numDif: %f\n", numDif);
            printToLog("refinedNumDif: %f\n", refinedNumDif);
            printToLog("markerDiffs: %f\n", markerDiffs);
            // Save and update the models if the marker content changed enough
            if(markerDiffs > .008) {
                // Only save the image if a meaningful has been stored
                if(realImageIsStored) {
                    Mat enhancedImage = PAOLProcUtils::enhanceColor(oldRectified);
                    //Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
                    Mat oldRefinedBackgroundSmooth = PAOLProcUtils::darkenText(currentMarkerModel,enhancedImage);
                    //Mat oldRefinedBackgroundSmooth = PAOLProcUtils::CLAHE(oldRectified);
                    Mat oldRefinedBackgroundSmooth2 = PAOLProcUtils::rectifyImage(oldRefinedBackgroundSmooth,corners);
                    //Mat oldRefinedBackgroundSmooth2 = PAOLProcUtils::findMarkerWithMarkerBorders(oldRectified);
                    //Mat oldRefinedBackgroundSmooth2 = PAOLProcUtils::CLAHE(oldRectified);
                    //Mat oldRefinedBackgroundSmooth = PAOLProcUtils::darkenText(pdrift,oldRefinedBackgroundSmooth2);
                    //Mat enhancedImage = PAOLProcUtils::enhanceColor(oldRectified);
                    saveImageWithTimestamp(oldRefinedBackgroundSmooth2);
                    //saveImageWithTimestamp(oldRectified);
                    //saveImageWithTimestamp(pdrift);
                }
                // Update marker model
                oldMarkerModel = currentMarkerModel.clone();
                // Update enhanced version of background
                Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentRectified, currentMarkerModelGrown);
                oldRefinedBackground = PAOLProcUtils::updateModel(
                            oldRefinedBackground, whiteWhiteboard, mvmtFullSize);
                realImageIsStored = true;
            }
        }
    }
    // Otherwise, check if the frames are basically identical (ie. stable)
    else if(numDif < .000001) {
        stableWhiteboardCount++;
        // If the image has been stable for exactly three frames, the lecturer is not present, so we
        // can update the marker and whiteboard models without movement information
        if(stableWhiteboardCount == 3) {
            // Only save the image if a meaningful has been stored
            if(realImageIsStored) {
                // Save the smooth marker version of the old background image
                Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
                saveImageWithTimestamp(oldRefinedBackgroundSmooth);
            }

            // Update marker model
            // Find marker candidates
            Mat markerCandidates = PAOLProcUtils::findMarkerStrokeCandidates(currentRectified);
            // Find marker locations
            Mat markerLocations = PAOLProcUtils::findMarkerStrokeLocations(currentRectified);
            // Keep marker candidates intersecting with marker locations
            Mat currentMarkerModel = PAOLProcUtils::filterConnectedComponents(markerCandidates, markerLocations);

            oldMarkerModel = currentMarkerModel.clone();
            // Update enhanced version of background
            Mat whiteWhiteboard = PAOLProcUtils::whitenWhiteboard(currentRectified, currentMarkerModel);
            oldRefinedBackground = whiteWhiteboard.clone();
            realImageIsStored = true;
        }
    }*/
}

void WhiteboardWorker::saveImageWithTimestamp(const Mat& image) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/whiteboard/whiteBoard" << currentImageTime << "-" << whiteboardNum << ".png";
    imwrite(ss.str(), image);

    // Print image save success
    printToLog("Saved picture from worker %p at time %ld\n", this, currentImageTime);
    // Let listeners know that an image was processed
    emit savedImage(image);

    // Increment number of saved images
    saveImageCount++;
}

void WhiteboardWorker::saveImageWithTimestamp(const Mat& image,string name) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/whiteboard/whiteBoard" << name << currentImageTime << "-" << whiteboardNum << ".png";
    imwrite(ss.str(), image);

    // Print image save success
    printToLog("   Saved named picture from worker %p at time %ld\n", this, currentImageTime);
    // Let listeners know that an image was processed
    //emit savedImage(image);

    // Increment number of saved images
    saveImageCount++;
}

void WhiteboardWorker::printToLog(char *format, ...) {
    va_list argptr;
    va_start(argptr, format);
    vfprintf(logFile, format, argptr);
    va_end(argptr);
}

// Parses the corners from the wbCorners#.txt file, where # is the whiteboard index
// that this worker is processing. The format should have four lines, each with the
// comma-separated tuple that defines the corner. For example, the default corners
// would correspond a wbCorners#.txt file with the following content:
//      0, 0
//      1920, 0
//      1920, 1080
//      0, 1080
WBCorners WhiteboardWorker::getCornersFromFile(int wbNum) {
    WBCorners ret;

    // Try to open the file
    stringstream ss;
    int loc=QDir::currentPath().toStdString().find("/paol-code/");
    std::string pathTemp=QDir::currentPath().toStdString();
    std::string codePath=pathTemp.substr(0,loc)+"/paol-code/wbCorners";
    ss << codePath.c_str() << wbNum << ".txt";
    //qDebug("complete screwup wbNum=%d",wbNum);
    //qDebug() << QString::fromStdString(ss.str());
    QFile file(QString::fromStdString(ss.str()));
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        //qDebug("found corners file");
        // Corners file was opened, so get the file text
        QTextStream fileStream(&file);
        QString fileText = fileStream.readAll();
        // Split file text by commas and whitespace
        QStringList coordinates = fileText.split(QRegExp("\n|\r\n|\r|,"));
        assert(coordinates.size() >= 8);
        // Set coordinates
        ret.TL = Point2f(coordinates[0].toInt(), coordinates[1].toInt());
        ret.TR = Point2f(coordinates[2].toInt(), coordinates[3].toInt());
        ret.BR = Point2f(coordinates[4].toInt(), coordinates[5].toInt());
        ret.BL = Point2f(coordinates[6].toInt(), coordinates[7].toInt());
        // Sort the corners in case they are in the wrong order
        PAOLProcUtils::sortCorners(ret);
    }
    else {
        // Corners file was not found, so set ret to default coordinates
        ret.TL = Point2f(0, 0);
        ret.TR = Point2f(1920, 0);
        ret.BR = Point2f(1920, 1080);
        ret.BL = Point2f(0, 1080);
    }
    return ret;
}

void WhiteboardWorker::saveLastImage() {
    if(realImageIsStored) {
        // Save the smooth marker version of the old background image
        //Mat oldRefinedBackgroundSmooth = PAOLProcUtils::smoothMarkerTransition(oldRefinedBackground);
        saveImageWithTimestamp(oldRefinedBackground);
    }
}

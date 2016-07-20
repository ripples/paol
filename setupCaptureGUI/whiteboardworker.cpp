#include "whiteboardworker.h"

WhiteboardWorker::WhiteboardWorker(string deviceUSB, int camNumIn, int wbNum, bool camFlipped, string lecPath, bool guiRunning)
{
    runningGUI=guiRunning;

    // Initialize webcam and associated variables
    camera = VideoCapture(camNumIn);
    camera.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    camera.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
    flipCam = camFlipped;
    deviceNum = camNumIn;
    whiteboardNum = wbNum;
    USB = deviceUSB;

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

    corners = getCornersFromFile(deviceUSB);

    // Print the association between this process and the output
    printToLog("WB %d: %p\n", whiteboardNum, this);

    consecutiveStableCount=10;
    changedBoard=true;
}

bool WhiteboardWorker::takePicture() {
    // Set old frame
    //oldRefined = currentFrame.clone();

    // Set current frame (grab five times so we get actual current frame)
    for(int i = 0; i < 5; i++){
        if(currentFrame.data)
            currentFrame.release();
        camera >> currentFrame;
    }
    // Flip the image horizontally and vertically if the camera is upside-down
    if(flipCam) {
        flip(currentFrame, currentFrame, -1);
    }
    if(currentFrame.data) {
        // Image capture succeeded
        // Let listeners know that an image was captured
        if(currentRectified.data)
            currentRectified.release();
        currentRectified = PAOLProcUtils::rectifyImage(currentFrame, corners);
        if(runningGUI)
            emit capturedImage(currentRectified);
        return true;
    }
    else {
        // Image capture failed
        return false;
    }
}

void WhiteboardWorker::processImage() {
    double correlation=.707;
    int nccWindowSize=20;
    int framesToBeStable=10;

    //moved to take picture
    //Mat currentRectified = PAOLProcUtils::rectifyImage(currentFrame, corners);

    //get ground truth for whiteboard by bluring image
    if(blured.data)
        blured.release();
    blur(currentRectified,blured,Size(10,10));

    //If more then one picture has been taken create normalized cross correlation image between frames
    if(oldRectified.data){
        //save the old normalized cross correlation image if it exists
        if(normalizedCrossCorrelation.data){
            if(oldNormalizedCrossCorrelation.data)
                oldNormalizedCrossCorrelation.release();
            oldNormalizedCrossCorrelation=normalizedCrossCorrelation.clone();
        }
        //run normlized cross correlation
        if(normalizedCrossCorrelation.data)
            normalizedCrossCorrelation.release();
        normalizedCrossCorrelation=PAOLProcUtils::NCC(oldRectified,currentRectified,correlation,nccWindowSize);
    }

    //create old image late enough so that ncc doesn't run till second pass
    if(currentRectified.data){
        if(oldRectified.data)
            oldRectified.release();
        oldRectified=currentRectified.clone();
    }

    //if the program has run long enough to have data in all images
    if (oldNormalizedCrossCorrelation.data){
        //initialize stable image to zeros if it does not exist
        if(!stablePixels.data)
            stablePixels=Mat::zeros(currentRectified.size(),currentRectified.type());

        //determine which pixels are stable
        PAOLProcUtils::StablePix(stablePixels,oldNormalizedCrossCorrelation,normalizedCrossCorrelation,26);

        //enhance text and whiten whiteboard
        if(refinedCurrentFrame.data)
            refinedCurrentFrame.release();
        refinedCurrentFrame=PAOLProcUtils::refineImage(currentRectified,blured);

        //intialize images
        if(!currentWhiteboard.data){
            currentWhiteboard=refinedCurrentFrame.clone();
        }

        if(oldWhiteboard.data)
            oldWhiteboard.release();
        oldWhiteboard=refinedCurrentFrame.clone();
;
        //if the program has run enough for these images to exist
        if(notWhiteboardEroded.data){
            if(oldNotWhiteboardEroded.data)
                oldNotWhiteboardEroded.release();
            oldNotWhiteboardEroded=notWhiteboardEroded.clone();
        }

        //update whiteboard in stable locations
        PAOLProcUtils::updateBackground(currentWhiteboard,currentRectified,stablePixels,framesToBeStable,refinedCurrentFrame);

        //identify areas that are not whiteboard
        if(notWhiteboard.data)
            notWhiteboard.release();
        notWhiteboard=PAOLProcUtils::getNotWhite(currentWhiteboard);

        //these next two steps work together to fill in holes
        //grow the areas that are not whiteboard
        if(notWhiteboardGrown.data)
            notWhiteboardGrown.release();
        notWhiteboardGrown=PAOLProcUtils::growGreen(notWhiteboard,2);
        //erode the areas that are not whiteboard and at the edges just grown
        if(notWhiteboardEroded.data)
            notWhiteboardEroded.release();
        notWhiteboardEroded=PAOLProcUtils::erodeSizeGreen(notWhiteboardGrown,7);

        //if multiple images that have identified areas of text exist
        if(oldNotWhiteboardEroded.data){
            //find the differences in text regions
            if(notWhiteboardDifference.data)
                notWhiteboardDifference.release();
            notWhiteboardDifference=PAOLProcUtils::getErodeDifferencesIm(oldNotWhiteboardEroded,notWhiteboardEroded);
            //erode those difference areas to try to remove noise
            if(notWhiteboardDifference2.data)
                notWhiteboardDifference2.release();
            notWhiteboardDifference2=PAOLProcUtils::erodeSize(notWhiteboardDifference,4);

            //count the number of differences still present
            currentDifference=PAOLProcUtils::countDifferences(notWhiteboardDifference2);
            if(changedBoard){
                printToLog("currentDifferences: %d stable: %d changedBoard=true\n", currentDifference,consecutiveStableCount);
                qDebug("currentDifferences: %d stable: %d changedBoard=true\n", currentDifference,consecutiveStableCount);
            } else {
                printToLog("currentDifferences: %d stable: %d changedBoard=false\n", currentDifference,consecutiveStableCount);
                qDebug("currentDifferences: %d stable: %d changedBoard=false\n", currentDifference,consecutiveStableCount);
            }

            //if the number of differences is greater then 20 (a large change) reset the stable count and say that there is a change
            if(currentDifference>20){
                changedBoard=true;
                consecutiveStableCount=0;//probably not necessary with the next if statement.
            }

            //if they are identical then say they are stable else reset the stable counter
            if(currentDifference==0){
                consecutiveStableCount++;
            }else{
                consecutiveStableCount=0;
            }

            //if stable (more then 5 the same) and a change occurs then save
            if(consecutiveStableCount>=5 && changedBoard){
                saveImageWithTimestamp(currentWhiteboard);
            }
        }
    }
}


void WhiteboardWorker::saveImageWithTimestamp(const Mat& image) {
    // Construct the path to save the image
    stringstream ss;
    ss << lecturePath << "/whiteboard/whiteBoard" << "-" << whiteboardNum << "-" << currentImageTime << ".png";
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
    ss << lecturePath << "/whiteboard/whiteBoard" << name << "-" << whiteboardNum << "-" << currentImageTime << ".png";
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
WBCorners WhiteboardWorker::getCornersFromFile(string deviceUSB) {
    WBCorners ret;

    // Try to open the file
    stringstream ss;
    int loc=QDir::currentPath().toStdString().find("/paol-code/");
    std::string pathTemp=QDir::currentPath().toStdString();
    std::string codePath=pathTemp.substr(0,loc)+"/paol-code/wbCorners";
    ss << codePath.c_str() << deviceUSB << ".txt";
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
        file.close();
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

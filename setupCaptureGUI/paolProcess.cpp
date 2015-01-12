#include "paolProcess.h"

#define bottomMask 115
#define thresholdDiff .0002
#define repeat 3//number of consecutive stable images necessary to consider stable

paolProcess::paolProcess(int camNumIn, QLabel &locationIn, QLabel &locationOut, bool whiteboardIn, string pathIn) : QThread(){
    locIn=&locationIn;
    locOut=&locationOut;
    whiteboard=whiteboardIn;

    secondsElapsed = 0;
    frameCount = 0;

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

    count=0;
    picNum = 0;
    camNum = camNumIn;
    outputPath = pathIn;
    record = true;

    if(whiteboard){
        outputPath = outputPath + "/wboard";
        string command = "mkdir -p " + outputPath;
        system(command.data());
        outputPath = outputPath + "/";
    }
    else{
        outputPath = outputPath + "/computer";
        string command = "mkdir -p " + outputPath;
        system(command.data());
        outputPath = outputPath + "/";
    }
}

paolProcess::~paolProcess(){
    qDebug() << "Camera " << camNum << " has taken " << frameCount << " frames.";
    if(camera.isOpened()) {
        camera.release();
    }
}

void paolProcess::run(){
    //while(1){
        callTakePicture();
        process();
        displayInput();
        displayOutput();
    //}
}



void paolProcess::callTakePicture(){
    if(whiteboard) {
        for(int i = 0; i < 5; i++) {
            camera >> currentFrame;
        }
    }
    else {
        for(int i = 0; i < 5; i++) {
            camera >> currentScreen;
        }
    }
    frameCount++;
}

void paolProcess::process(){
    if (whiteboard == true){
        if(record == true){
            processWB();
        }
    }
    else{
        if(record == true){
            processComp();
        }
    }
}

void paolProcess::flipRecord(){
    record = !record;
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
        count=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
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

            //SAVES OUT IMAGE
            //Convert picNum to string
            stringstream ss;
            ss << picNum;
            string str = ss.str();

            //Convert camNum to string
            stringstream ss2;
            ss2 << camNum;
            string str2 = ss2.str();

            string s;
            stringstream out;
            //Create string that has image#
            out << setfill ('0') << setw (6);
            out << str << endl;
            s = out.str();

            //Get system time
            // TODO: The time retrieved here doesn't reflect the time the actual image was captured.
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,80,"%H%M%S",timeinfo);

            string destination = outputPath + s + "-" + buffer + "-" + str2 + ".png";
            imwrite(destination.data(), oldRefinedBackground);

            picNum++;
        }
        //copy last clean whiteboard image
        oldRefinedBackground = currentWhiteboardModel.clone();
        oldMarkerModel = newMarkerModel.clone();
    }
    oldFrame = currentFrame.clone();
}

void paolProcess::processComp(){
    float percentDifference = WhiteboardProcessor::difference(oldScreen, currentScreen);

    printf("perDif=%f\n",percentDifference);
    //if percentDifference is greater than the threshold
    if(percentDifference>=thresholdDiff){
        //printf(" perDif=%f thres=%f\n",percentDifference,thresholdDiff);
        //then if the number of identical images is greater than or equal to 3
        if (countStable>=repeat){
            //SAVES OUT IMAGE
            //Convert picNum to string
            stringstream ss;
            ss << picNum;
            string str = ss.str();

            //Convert camNum to string
            stringstream ss2;
            ss2 << camNum;
            string str2 = ss2.str();

            string s;
            stringstream out;
            //Create string that has image#
            out << setfill ('0') << setw (6);
            out << str << endl;
            s = out.str();

            //Get system time
            // TODO: Get the system time when the image is captured
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,80,"%H%M%S",timeinfo);

            string destination = outputPath + s + "-" + buffer + "-" + str2 + ".png";
            lastStableScreen = oldScreen.clone();
            imwrite(destination.data(), lastStableScreen);
            picNum++;
        }

        countStable=0;
    } else {
        countStable++;
    }
    oldScreen = currentScreen.clone();
}

void paolProcess::displayInput(){
    if(whiteboard) {
        displayMat(currentFrame, *locIn);
    }
    else {
        displayMat(currentScreen, *locIn);
    }
}

void paolProcess::displayWB(){
    displayMat(oldRefinedBackground, *locOut);
}

void paolProcess::displayComp(){
    displayMat(lastStableScreen, *locOut);
}

void paolProcess::displayOutput(){
    if (whiteboard){
        displayWB();
    }
    else{
        displayComp();
    }
}

QImage paolProcess::convertMatToQImage(const Mat& mat) {
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mat,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888)
            .copy();
    return img;
}

void paolProcess::displayMat(const Mat& mat, QLabel &location) {
    //call method to convert Mat to QImage
    QImage img=convertMatToQImage(mat);
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

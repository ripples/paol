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
    cam=new paolMat();
    old=new paolMat();
    background=new paolMat();
    backgroundRefined=new paolMat();
    oldBackgroundRefined=new paolMat();
    rawEnhanced=new paolMat();

    cam->setCameraNum(camNumIn);
    cam->takePicture();
    //camRaw->setCameraNum(camNumIn);
    //camRaw->takePicture();
    //cam->copyClean(camRaw);
    //cam->rectifyImage(camRaw);
    old->copy(cam);
    if(whiteboard) {
        oldFrame = cam->src.clone();
        oldRefinedBackground = Mat::zeros(oldFrame.size(), oldFrame.type());
        oldMarkerModel = Mat::zeros(oldFrame.size(), oldFrame.type());
    }
    else {
        oldScreen = cam->src.clone();
    }
    background->copyClean(cam);
    backgroundRefined->copyClean(cam);
    oldBackgroundRefined->copyClean(cam);
    rawEnhanced->copyClean(cam);

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
    cam->~paolMat();
    old->~paolMat();
    background->~paolMat();
    backgroundRefined->~paolMat();
    oldBackgroundRefined->~paolMat();
    rawEnhanced->~paolMat();
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
    cam->takePicture();
    frameCount++;
    //camRaw->takePicture();
    //cam->copyClean(camRaw);
    //cam->rectifyImage(camRaw,corners);
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
    Mat currentFrame = cam->src.clone();
    //compare picture to previous picture and store differences in old->maskMin
    float numDif;
    Mat allDiffs;
    WhiteboardProcessor::findAllDiffsMini(allDiffs, numDif, oldFrame, currentFrame, 40, 1);

    //if there is enough of a difference between the two images
    float refinedNumDif = 0;
    Mat filteredDiffs = Mat::zeros(currentFrame.size(), currentFrame.type());
    if(numDif>.03){
        //set up a new % that represents difference
        WhiteboardProcessor::filterNoisyDiffs(filteredDiffs, refinedNumDif, allDiffs);
        count=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
        //copy the input image and process it to highlight the text
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

        //count the number of differences in the refined text area between refined images
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
    if(old->src.rows==cam->src.rows && old->src.cols==cam->src.cols){
        cam->difference(old, 100, 0, bottomMask);
        //percentDifference is (all the differences)/(size of the image)
        percentDifference=(double)cam->difs/(double)(cam->src.rows*cam->src.cols);
    } else {
        percentDifference=1;
    }
    printf("perDif=%f\n",percentDifference);
    //if percentDifference is greater than the threshold
    if(percentDifference>=thresholdDiff){
        //printf(" perDif=%f thres=%f\n",percentDifference,thresholdDiff);
        //then if the number of identical images is greater then or equal to 3
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
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];

            time (&rawtime);
            timeinfo = localtime(&rawtime);

            strftime(buffer,80,"%H%M%S",timeinfo);

            string destination = outputPath + s + "-" + buffer + "-" + str2 + ".png";
            oldBackgroundRefined->copy(old);
            imwrite(destination.data(), old->src);
            picNum++;
        }

        countStable=0;
        timeDif=cam->time;
    } else {
        countStable++;
    }
    old->copy(cam);
}

void paolProcess::displayInput(){
    cam->displayImage(*locIn);
}

void paolProcess::displayWB(){
    displayMat(oldRefinedBackground, *locOut);
}

void paolProcess::displayComp(){
    oldBackgroundRefined->displayImage(*locOut);
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

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
    //compare picture to previous picture and store differences in old->maskMin
    numDif=old->differenceMin(cam,40,1);

    //if there is enough of a difference between the two images
    if(numDif>.03){
        //set up a new % that represents difference
        refinedNumDif=old->shrinkMaskMin();
        count=0;
    } else {
        refinedNumDif=0;
    }

    //if the images are really identical, count the number of consecultive nearly identical images
    if (numDif < .000001)
        count++;

    //if the differences are enough that we know where the lecturer is or the images have been identical
    //for two frames, and hence no lecturer present
    if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
        //copy the input image and process it to highlight the text
        rawEnhanced->copy(cam);
        Mat markerLocation = paolMat::findMarkerWithCC(rawEnhanced->src);
        Mat darkenedText = paolMat::whitenWhiteboard(rawEnhanced->src, markerLocation);
        rawEnhanced->src = darkenedText.clone();

        /////////////////////////////////////////////////////////////
        //identify where motion is

        //extend the area of differences and sweep differences for more solid area
        old->extendMaskMinToEdges();
        old->sweepDownMin();
        //keep only the solid area and grow that region
        old->keepWhiteMaskMin();
        old->growMin(8);
        //draw a convex hull around area of differences
        old->findContoursMaskMin();
        //fill in area surrounded by convex hull
        old->sweepDownMin();
        old->keepWhiteMaskMin();
        ///////////////////////////////////////////////////////////////////////

        //process to identify text location

        //smooth image
        cam->blur(1);
        //find edge information and store total edge information in 0 (blue) color channel of mask
        cam->pDrift();
        //grow the area around where the edges are found (if edge in channel 0 grow in channel 2)
        cam->grow(15,3);
        ////////////////////////////////////////////////////////////////////////////////

        //process to update background image

        //copy movement information into rawEnhanced and then expand to full mask
        rawEnhanced->copyMaskMin(old);
        rawEnhanced->maskMinToMaskBinary();

        //update the background image with new information
        background->updateBack2(rawEnhanced,cam);

        //copy the background image to one for processing
        backgroundRefined->copy(background);
        //copy text location information into mask
        backgroundRefined->copyMask(background);
        //////////////////////////////////////////////////

        //figure out if saves need to be made

        //count the number of differences in the refined text area between refined images
        saveNumDif = oldBackgroundRefined->countDifsMask(backgroundRefined);
        if (saveNumDif>.004){
            oldBackgroundRefined->time=old->time;

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
            imwrite(destination.data(), oldBackgroundRefined->src);

            picNum++;
        }
        //copy last clean whiteboard image
        oldBackgroundRefined->copy(backgroundRefined);
    }
    old->copy(cam);
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
    oldBackgroundRefined->displayImage(*locOut);
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


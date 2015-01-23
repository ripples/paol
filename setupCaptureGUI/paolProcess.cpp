#include "paolProcess.h"

paolProcess::paolProcess() {
    keepRunning = true;
}

paolProcess::~paolProcess() {
    if(camera.isOpened())
        camera.release();
    if(logFile != NULL) {
        fclose(logFile);
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
    printToLog("Stopped thread %p after capturing %d frames and saving %d frames\n",
           this, capturedImageCount, saveImageCount);
}

void paolProcess::onQuitProcessing() {
    printToLog("Received signal to stop thread %p\n", this);
    keepRunningMutex.lock();
    keepRunning = false;
    keepRunningMutex.unlock();
}



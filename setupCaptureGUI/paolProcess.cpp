#include "paolProcess.h"

paolProcess::~paolProcess() {
    delete worker;
    delete timer;
    delete thread;
}

void paolProcess::run(){

    thread->start();
    while(1){
        // Stop thread if keepRunning is false
        keepRunningMutex.lock();
        if(!keepRunning) {
            keepRunningMutex.unlock();
            thread->quit();
            worker->writeFinishStatistics();
            break;
        }
        keepRunningMutex.unlock();
    }
}

void paolProcess::onQuitProcessing() {
    keepRunningMutex.lock();
    keepRunning = false;
    keepRunningMutex.unlock();
}

void paolProcess::onImageSaved(const Mat &image) {
    emit savedImage(image, this);
}

void paolProcess::onImageCaptured(const Mat &image) {
    emit capturedImage(image, this);
}

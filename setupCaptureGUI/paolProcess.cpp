#include "paolProcess.h"

paolProcess::paolProcess() {
    keepRunning = true;
}

paolProcess::~paolProcess() {
    if(camera.isOpened())
        camera.release();
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
    qDebug("Stopped thread %p after capturing %d frames and saving %d frames",
           this, capturedImageCount, saveImageCount);
}

void paolProcess::onQuitProcessing() {
    qDebug("Received signal to stop thread %p", this);
    keepRunningMutex.lock();
    keepRunning = false;
    keepRunningMutex.unlock();
}

void paolProcess::saveImageWithTimestamp(const Mat& image) {
    // Get the path to save the image to (ie. fill in the timestamp)
    char destination[256];
    sprintf(destination, saveImagePathFormat, currentImageTime);
    // Save the image
    imwrite(destination, image);

    // Print image save success
    qDebug("Saved picture in thread %p at time %ld", this, currentImageTime);
    // Let listeners know that an image was processed
    emit savedImage(image, this);

    // Increment number of saved images
    saveImageCount++;
}

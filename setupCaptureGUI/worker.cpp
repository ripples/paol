#include "worker.h"

Worker::Worker() {
    realImageIsStored = false;
}

Worker::~Worker() {
    if(camera.isOpened())
        camera.release();
    if(logFile != NULL)
        fclose(logFile);
}

void Worker::finish() {
    saveLastImage();
    writeFinishStatistics();
}

void Worker::writeFinishStatistics() {
    printToLog("Stopped thread %p after capturing %d frames and saving %d frames\n",
               this, capturedImageCount, saveImageCount);
}

void Worker::workOnNextImage() {
    bool gotPicture = takePicture();
    // Update time associated with current frame
    time(&currentImageTime);
    if(gotPicture) {
        // Print image capture success
        printToLog("Took picture in thread %p at time %ld\n", this, currentImageTime);
        // Increase captured image count
        capturedImageCount++;

        processImage();
    }
    else {
        // Print image capture failure
        printToLog("Failed to take picture in thread %p at time %ld\n", this, currentImageTime);
    }
}

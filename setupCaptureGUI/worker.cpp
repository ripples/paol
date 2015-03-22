#include "worker.h"

Worker::~Worker() {
    if(camera.isOpened())
        camera.release();
    if(logFile != NULL)
        fclose(logFile);
}

void Worker::writeFinishStatistics() {
    printToLog("Stopped thread %p after capturing %d frames and saving %d frames\n",
               this, capturedImageCount, saveImageCount);
}

void Worker::workOnNextImage() {
    bool gotPicture = takePicture();
    if(gotPicture) {
        // Print image capture success
        printToLog("Took picture in thread %p at time %ld\n", this, currentImageTime);
        // Let listeners know that an image was captured
        emit capturedImage(currentFrame);
        // Increase captured image count
        capturedImageCount++;

        processImage();
    }
}

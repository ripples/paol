#include "paolProcess.h"

paolProcess::paolProcess() {
    // Initialize thread and timer
    workerThread = new QThread();
    timer = new QTimer();
    // Initialize run flag
    keepRunning = true;
}

paolProcess::~paolProcess() {
    delete worker;
    delete timer;
    delete workerThread;
}

void paolProcess::run(){
    // Start inner thread (i.e. start processing)
    workerThread->start();
    while(1){
        // Stop thread if keepRunning is false
        keepRunningMutex.lock();
        if(!keepRunning) {
            keepRunningMutex.unlock();
            break;
        }
        keepRunningMutex.unlock();
    }
    // Stop inner thread and wait for it to terminate
    workerThread->quit();
    workerThread->wait();
    // Do the last step
    worker->finish();
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

void paolProcess::setupThreadWorkerTimer() {
    // Connect image captured and saved slots to worker
    connect(worker, SIGNAL(capturedImage(Mat)), this, SLOT(onImageCaptured(Mat)));
    connect(worker, SIGNAL(savedImage(Mat)), this, SLOT(onImageSaved(Mat)));

    // Connect processing worker to timer
    connect(timer, SIGNAL(timeout()), worker, SLOT(workOnNextImage()));
    // Set timer to fire off every second
    timer->start(1000);
    // Make timer and worker run on a different thread
    timer->moveToThread(workerThread);
    worker->moveToThread(workerThread);
}

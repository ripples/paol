#include "vgaprocess.h"

VGAProcess::VGAProcess(int camNumIn, int compNum, bool camFlipped, string lecPath)
{
    // Initialize thread, timer, and worker
    thread = new QThread();
    timer = new QTimer();
    worker = new VGAWorker(camNumIn, compNum, camFlipped, lecPath);

    // Connect image captured and saved slots to worker
    connect(worker, SIGNAL(capturedImage(Mat)), this, SLOT(onImageCaptured(Mat)));
    connect(worker, SIGNAL(savedImage(Mat)), this, SLOT(onImageSaved(Mat)));

    // Connect processing worker to timer
    connect(timer, SIGNAL(timeout()), worker, SLOT(workOnNextImage()));
    // Set timer to fire off every second
    timer->start(1000);
    // Make timer and worker run on a different thread
    timer->moveToThread(thread);
    worker->moveToThread(thread);

    keepRunning = true;
}

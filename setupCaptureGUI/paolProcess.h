#ifndef PAOLPROCESS_H
#define PAOLPROCESS_H

#include <QtCore>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "worker.h"

using namespace cv;
class paolProcess : public QThread {
    Q_OBJECT

protected:
    // Boolean and associated mutex to indicate whether we should keep processing
    QMutex keepRunningMutex;
    volatile bool keepRunning;

    // Timer to capture once per second
    QTimer* timer;
    // Thread to do the stuff
    QThread* thread;
    // Worker to do the stuff
    Worker* worker;

public:
    ~paolProcess();
    void run();

signals:
    void capturedImage(Mat image, paolProcess* threadAddr);
    void savedImage(Mat image, paolProcess* threadAddr);

private slots:
    void onQuitProcessing();
    void onImageSaved(const Mat& image);
    void onImageCaptured(const Mat& image);
};

#endif // PAOLPROCESS_H

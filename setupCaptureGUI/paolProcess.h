#ifndef PAOLPROCESS_H
#define PAOLPROCESS_H

#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <QtCore/QThread>
#include <QLabel>
#include <QString>
#include <QMainWindow>
#include <QtCore>
#include <QImage>

#include <math.h>

#include "WhiteboardProcessor.h"

using namespace std;
class paolProcess : public QThread {
    Q_OBJECT

private:
    // Boolean and associated mutex to indicate whether we should keep processing
    QMutex keepRunningMutex;
    volatile bool keepRunning;

protected:
    // Webcam and associated properties
    VideoCapture camera;
    bool flipCam;
    int deviceNum;

    // Fields for general processing (mostly saving the image)
    int saveImageCount;
    char saveImagePathFormat[256];
    time_t currentImageTime;
    int capturedImageCount; // How many images were captured from the camera

    // Methods for general processing
    virtual void workOnNextImage() = 0;
    virtual bool takePicture() = 0;
    virtual void processImage() = 0;

    // Method to save an image at the current time
    void saveImageWithTimestamp(const Mat& image);

public:
    paolProcess();
    ~paolProcess();
    void run();

signals:
    void capturedImage(Mat image, paolProcess* threadAddr);
    void processedImage(Mat image, paolProcess* threadAddr);

private slots:
    void onQuitProcessing();
};

#endif // PAOLPROCESS_H

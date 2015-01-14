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

public:
    // Constants for processing computer image differences
    static const float COMP_DIFF_THRESHOLD = .0002;
    static const int COMP_REPEAT_THRESHOLD = 3;

    // Boolean and associated mutex to indicate whether we should keep processing
    QMutex keepRunningMutex;
    volatile bool keepRunning;

    // Device to get frames
    VideoCapture camera;

    // Fields for general processing
    int saveImageCount;
    int deviceNum;
    int capturedImageCount; // How many images were captured from the camera
    char saveImagePathFormat[256];
    time_t currentImageTime;

    // Fields for whiteboard processing
    Mat currentFrame;
    Mat oldFrame;
    Mat oldMarkerModel;
    Mat oldRefinedBackground; // What the whiteboard from the oldFrame looks like
    int stableWhiteboardCount;

    // Fields for computer processing
    Mat currentScreen;
    Mat oldScreen;
    Mat lastStableScreen;
    int stableScreenCount;

    // Tag
    bool whiteboard;

    paolProcess(int camNumIn, bool whiteboardIn, string pathIn);
    ~paolProcess();
    void callTakePicture();
    void processWB();
    void processComp();
    void workOnNextImage();

protected:
    void run();

signals:
    void capturedImage(Mat image, paolProcess* threadAddr);
    void processedImage(Mat image, paolProcess* threadAddr);

private slots:
    void onQuitProcessing();
};

#endif // PAOLPROCESS_H

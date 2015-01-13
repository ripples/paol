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
    // Boolean and associated mutex to indicate whether we should keep processing
    QMutex keepRunningMutex;
    volatile bool keepRunning;

    // Device to get frames
    VideoCapture camera;

    // Fields for whiteboard processing
    Mat currentFrame;
    Mat oldFrame;
    Mat oldMarkerModel;
    Mat oldRefinedBackground;

    // Fields for computer processing
    Mat currentScreen;
    Mat oldScreen;
    Mat lastStableScreen;

    float numDif,refinedNumDif,saveNumDif;
    int count;
    bool whiteboard;

    string outputPath;

    double percentDifference;
    int countStable;
    int timeDif;
    int picNum;
    int camNum;

    int secondsElapsed;
    int frameCount;

    bool record;
    void flipRecord();

    paolProcess(int camNumIn, bool whiteboardIn, string pathIn);
    ~paolProcess();
    void callTakePicture();
    void processWB();
    void processComp();
    void process();

protected:
    void run();

signals:
    void capturedImage(Mat image, paolProcess* threadAddr);
    void processedImage(Mat image, paolProcess* threadAddr);

private slots:
    void onQuitProcessing();
};

#endif // PAOLPROCESS_H

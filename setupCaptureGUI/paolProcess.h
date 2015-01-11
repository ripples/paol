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

#include "paolMat.h"
#include "WhiteboardProcessor.h"

using namespace std;
class paolProcess : public QThread
{
public:
    // Fields for whiteboard processing
    Mat oldFrame;
    Mat oldMarkerModel;
    Mat oldRefinedBackground;

    // Fields for computer processing
    Mat oldScreen;

    Ptr<paolMat> cam;
    Ptr<paolMat> camRaw;
    Ptr<paolMat> old;
    Ptr<paolMat> background;
    Ptr<paolMat> backgroundRefined;
    Ptr<paolMat> oldBackgroundRefined;
    Ptr<paolMat> rawEnhanced;
    QLabel* locIn;
    QLabel* locOut;

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

    paolProcess(int camNumIn, QLabel& locationIn, QLabel& locationOut, bool whiteboardIn, string pathIn);
    ~paolProcess();
    void callTakePicture();
    void processWB();
    void processComp();
    void process();
    void displayInput();
    void displayWB();
    void displayComp();
    void displayOutput();

    QImage convertMatToQImage(const Mat& mat);
    void displayMat(const Mat& mat, QLabel &location);

protected:
    void run();
};

#endif // PAOLPROCESS_H

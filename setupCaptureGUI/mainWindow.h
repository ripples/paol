#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QTimer>
#include <QTime>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>

#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include <sstream>
#include <fstream>

#include <stdio.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <iomanip>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "paolMat.h"
#include "paolProcess.h"
#include "unistd.h"

using namespace std;
using namespace cv;
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QVector <paolProcess*> dev;

    QVector <VideoCapture> recordingCams;

    QVector <QLabel*> camLabels;
    QVector <QLabel*> imLabels; //Vector containing pointers to all Labels
    QVector <QLabel*> paolLabels;
    QVector <QLabel*> previewLabels;

    QVector <QLayout*> setupLayouts;
    QVector <QLayout*> capLayouts;

    QVector <QComboBox*> optionBoxes; //Vector containing pointers to all ComboBoxes
    QVector <QCheckBox*> reverseChecks;
    QVector <QRadioButton*> audioRecord;

    QVector <QString> camTypes;

    QTime myTimer;

    string processLocation; //Contains location where to store processed images
    string audioCamNum;
    string vidCaptureString;

    int count; //Stores amount of cameras currently connected
    int fullCount;
    int captureCount;
    int captureSecondsElapsed;

    bool runSetupCams;
    bool runCaptureCams;
    bool isVideo;

    bool courseInformation();

    void findCameras();
    void populateSetupWindow();
    void populateCaptureWindow();
    void createCamDocument();
    void configureCaptureSettings();
    void timer();

    void releaseSetupElements();
    void releaseCaptureElements();

private slots:
    void runSystem(); //Continously runs the recording system
    void on_infoReturnToSetup_clicked();

    void on_setupRefreshCameras_clicked();

    void on_setupContinueToCapture_clicked();

    void on_infoContinue_clicked();

    void on_captureReturnToSetup_clicked();


    void on_setupUploadFiles_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

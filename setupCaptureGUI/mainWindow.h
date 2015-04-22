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

#include "paolProcess.h"
#include "whiteboardprocess.h"
#include "vgaprocess.h"
#include "unistd.h"

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // GENERAL VARIABLES
    int camCount; //Amount of connected cameras
    int whiteboards; //Amount of selected whiteboards
    int captureDevices; // Devices that are either WBs or VGA2USB
    bool continueToCapture;
    string processLocation;
    string vidCaptureString;
    bool videoCapture;

    map<paolProcess*, int> threadToUIMap;
    QVector <paolProcess*> dev;
    QVector <QLabel*> paolLabels;
    QVector <QLabel*> camLabels;

    // WHITEBOARD CORNER VARIABLES
    int corners_currentCam;
    int clicked;
    int tracker;
    int captureCount;
    Mat corners_Clone;
    QVector <Point> clickedCorners;
    QVector <Point> intersectionPoints;

    // VECTORS FOR SETUP
    QVector <QLayout*> setupLayouts;
    QVector <QLabel*> imLabels;
    QVector <VideoCapture> recordingCams;
    QVector <QComboBox*> optionBoxes;
    QVector <QLayout*> capLayouts;
    QVector <QCheckBox*> reverseChecks;
    QVector <QRadioButton*> audioRecord;

    // FUNCTIONS
    void countCameras();
    void populateSetupWindow();
    void populateCourseList();
    void populateCaptureWindow();
    void createInfoFile();
    void createCameraSetupFile();
    void createFileDirectory();
    void appendToCourse();
    void place_image();
    void createWBCornerTxt();
    Point determineIntersection(double, double,double,double,double,double,double,double);
    QImage convertMatToQImage(const Mat& mat);
    void displayMat(const Mat& mat, QLabel &location);

    void captureVideo();
    void releaseComponents();

private:
    Ui::MainWindow *ui;

private slots:
    void launch_System();
    void onImageCaptured(Mat image, paolProcess* threadAddr);
    void onImageSaved(Mat image, paolProcess* threadAddr);

    /// Main Menu Button Functions
    void on_mainMenu_Full_Run_clicked();
    void on_mainMenu_Setup_Cameras_clicked();
    void on_mainMenu_Upload_Lectures_clicked();

    /// Setup window button functions
    void on_setupReturnButton_clicked();
    void on_setupContinueButton_clicked();

    /// Whiteboard Corner button functions
    void Mouse_Pressed();

    void on_WBC_Return_Button_clicked();
    void on_WBC_Continue_Button_clicked();
    void on_WBC_PrevWB_clicked();
    void on_WBC_Clear_clicked();
    void on_WBC_NextWB_clicked();
    void on_captureLecture_Terminate_Button_clicked();
    void on_lecDet_Continue_Button_clicked();
    void on_lecDet_Previous_Button_clicked();
    void on_WBC_Save_clicked();

signals:
    void quitProcessing();
};

#endif // MAINWINDOW_H

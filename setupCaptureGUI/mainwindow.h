#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QtCore>
#include <QTimer>

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
#include <paolMat.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QTimer *qTimer;

    bool createdUI; //Boolean ensuring the GUI builds only once
    bool captureOn;

    float numDif;
    float refinedNumDif;
    float saveNumDif;
    double percentDifference;
    int countStable;
    int timeDif;

    vector <paolMat *> dev; //List containing paolMats
    vector <paolMat *> old;
    vector <paolMat *> background;
    vector <paolMat *> backgroundRefined;
    vector <paolMat *> oldBackgroundRefined;
    vector <paolMat *> rawEnhanced;

    vector <string> camType;
    vector <int> camNums;
    vector <string> camNames;

    QVector <QComboBox*> optionBoxes; //Vector containing pointers to all ComboBoxes
    QVector <QLabel*> imLabels; //Vector containing pointers to all Labels
    QVector <QLabel*> imLabels2;

    string outputInfo; //String containing information to be written to out file

    QString className; //Name of the class (ex. COMP171)
    QString classYear; //Year in which class is taking place (ex. SUMMER13)
    QString classDur; //Duration of class (ex. 2700 for 45min, 3600 for 1hr)

    int count; //interger containing amount of camera devices connected to device

private slots:
    void displayCam();
    void populateGUI();
    void processWhiteboards();
    void processComputer();

    void on_generateButton_clicked();
    void on_cameraSetupBttn_clicked();
    void on_beginCaptureBttn_clicked();
    void on_startCaptureBttn_clicked();
    void on_stopCaptureBttn_clicked();
};

#endif // MAINWINDOW_H

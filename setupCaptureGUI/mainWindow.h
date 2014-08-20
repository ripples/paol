#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QTimer>

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

    vector <paolMat *> dev; //List containing paolMats

    vector <string> camType;
    vector <int> camNums;
    vector <string> camNames;

    QVector <QLabel*> imLabels; //Vector containing pointers to all Labels
    QVector <QComboBox*> optionBoxes; //Vector containing pointers to all ComboBoxes
    QVector <QCheckBox*> reverseChecks;
    QVector <QRadioButton*> audioRecord;

    QString className; //Name of the class (ex. COMP171)
    QString classYear; //Year in which class is taking place (ex. SUMMER13)
    QString classDur; //Duration of class (ex. 2700 for 45min, 3600 for 1hr)

    int count; //interger containing amount of camera devices connected to device
    bool createdUI;
    string outputInfo; //String containing information to be written to out file

private slots:
    void populateGUI();
    void beginRun();
    void on_confirmBttn_clicked();
};

#endif // MAINWINDOW_H

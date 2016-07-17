#ifndef COMMANDLINETHREAD_H
#define COMMANDLINETHREAD_H

#include <QApplication>
#include <linux/videodev.h>
#include <QFile>
#include <QTextStream>
#include <string>
#include <vector>
#include <cassert>
#include <unistd.h>
#include <stdio.h>
#include "paolProcess.h"
#include "vgaprocess.h"
#include "whiteboardprocess.h"
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

typedef struct ptc ProcThreadConfig;
struct ptc {
    string deviceUSB;
    int deviceNum;
    string type;
    int typeNum;
    bool flipCam;
};

class CommandLineThread : public QThread {
    Q_OBJECT

private:
    vector<ProcThreadConfig> threadConfigs;
    vector<paolProcess*> procThreads;
    QProcess* ffmpegProcess;
    string ffmpegCommand;

    // Lecture information
    int lectureDuration;
    string lecturePath;
    string semester;
    string course;
    time_t startTime;
    string codePath;
    string processLocation;

    // Counts for the device types
    int whiteboardCount;
    int vgaCount;

    // Methods called to set up files at the start of recording
    string buildLecturePath(string semester, string course, time_t startTime);
    void makeDirectories();
    void setThreadConfigs(string configLocation, string deviceLocation);
    void createThreadsFromConfigs();
    void createUSBSetupFile(string cameraFile);
    void writeInfoFile();

public:
    explicit CommandLineThread(int argc, char** argv);
    ~CommandLineThread();

signals:
    // Signal to stop FFmpeg and processing threads
    void stopCapture();
    // Signal to let the main application know that this thread finished
    void finished();

private slots:
    void run();
};

#endif // COMMANDLINETHREAD_H

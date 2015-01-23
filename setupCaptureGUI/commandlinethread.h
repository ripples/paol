#ifndef COMMANDLINETHREAD_H
#define COMMANDLINETHREAD_H

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

using namespace std;

typedef struct ptc ProcThreadConfig;
struct ptc {
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

    // Counts for the device types
    int whiteboardCount;
    int vgaCount;

    // Handle to the log that will store the FFmpeg output
    FILE* ffmpegLog;

public:
    explicit CommandLineThread(int argc, char** argv);
    ~CommandLineThread();

    string buildLecturePath(string semester, string course, time_t startTime);
    void makeDirectories();
    void setThreadConfigs(string configLocation);
    void createThreadsFromConfigs();
    void writeInfoFile();

signals:
    // Signal to stop FFmpeg and processing threads
    void stopCapture();
    // Signal to let the main application know that this thread finished
    void finished();

private slots:
    void run();
    // Slot for capturing the output from FFmpeg
    void onFFmpegErrorOutput();
};

#endif // COMMANDLINETHREAD_H

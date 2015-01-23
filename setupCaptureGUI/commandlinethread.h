#ifndef COMMANDLINETHREAD_H
#define COMMANDLINETHREAD_H

#include <QFile>
#include <QTextStream>
#include <string>
#include <vector>
#include <cassert>
#include <unistd.h>
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

public:
    explicit CommandLineThread(int argc, char** argv);
    ~CommandLineThread();

    string buildLecturePath(string semester, string course, time_t startTime);
    void makeDirectories();
    bool setThreadConfigs(string configLocation);
    void createThreadsFromConfigs();
    void writeInfoFile();

signals:
    void stopCapture();
    void finished();

private slots:
    void run();
};

#endif // COMMANDLINETHREAD_H

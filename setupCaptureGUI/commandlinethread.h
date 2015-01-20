#ifndef COMMANDLINETHREAD_H
#define COMMANDLINETHREAD_H

#include <QFile>
#include <QTextStream>
#include <string>
#include <vector>
#include <cassert>
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

    int duration;
    string lecturePath;
    string ffmpegCommand;

public:
    explicit CommandLineThread(int argc, char** argv);
    ~CommandLineThread();

    string buildLecturePath(string semester, string course);
    bool setThreadConfigs(string configLocation);
    void createThreadsFromConfigs();
    void run();
};

#endif // COMMANDLINETHREAD_H

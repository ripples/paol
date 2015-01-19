#ifndef COMMANDLINETHREAD_H
#define COMMANDLINETHREAD_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <string>
#include <vector>
#include "paolProcess.h"

using namespace std;

typedef struct ptc ProcThreadConfig;
struct ptc {
    int deviceNum;
    string type;
    int typeNum;
    bool flipCam;
};

class CommandLineThread : public QObject {
    Q_OBJECT

private:
    vector<ProcThreadConfig> threadConfigs;
    vector<paolProcess*> procThreads;

public:
    explicit CommandLineThread(QObject *parent = 0);
    ~CommandLineThread();

    bool setThreadConfigs(string configLocation);
    void createThreadsFromConfigs();

signals:
    void finished();

public slots:
    void run();
};

#endif // COMMANDLINETHREAD_H

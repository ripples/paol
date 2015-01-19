#include "commandlinethread.h"

CommandLineThread::CommandLineThread(QObject *parent) : QObject(parent)
{

}

CommandLineThread::~CommandLineThread()
{

}

void CommandLineThread::run() {
    qDebug("Do the thing");

    setThreadConfigs("/home/paol/paol-code/cameraSetup.txt");
    createThreadsFromConfigs();

    // Finish
    emit finished();
}

bool CommandLineThread::setThreadConfigs(string configLocation) {
    // Reset the thread configurations array
    threadConfigs.clear();

    // Open the configuration file
    QString locAsQStr = QString::fromStdString(configLocation);
    QFile configFile(locAsQStr);
    if(!configFile.open(QIODevice::ReadOnly)) {
        qWarning("Failed to open the configuration file at %s", configLocation.c_str());
        return false;
    }

    // Initialize counts for how many whiteboards and VGA feeds there are
    int whiteboardCount = 0;
    int vgaCount = 0;

    // Read lines from the config file
    QTextStream in(&configFile);
    while(!in.atEnd()) {
        QString line = in.readLine();

        // Only parse non-empty lines
        if(line.length() > 0) {
            // Initialize fields to scan into
            int deviceNum;
            int flipCam;
            char type[16];
            int scanRes = sscanf(line.toStdString().data(), "%d %d %s", &deviceNum, &flipCam, type);

            // Make sure exactly three items were found on the current line
            if(scanRes != 3) {
                qWarning("The configuration file was not in the correct format.");
                threadConfigs.clear();
                return false;
            }

            // Set thread configuration struct for the current line
            ProcThreadConfig p;
            p.deviceNum = deviceNum;
            p.flipCam = flipCam;
            p.type = string(type);
            if(p.type == "Whiteboard") {
                p.typeNum = whiteboardCount;
                whiteboardCount++;
            }
            else if(p.type == "VGA2USB") {
                p.typeNum = vgaCount;
                vgaCount++;
            }

            // Add the configuration struct to the set of configs
            threadConfigs.push_back(p);
        }
    }

    // Make sure at least one configuration was created
    if(threadConfigs.size() > 0) {
        return true;
    }
    else {
        qWarning("No non-empty lines were found in the config file.");
        return false;
    }
}

void CommandLineThread::createThreadsFromConfigs() {
    for(unsigned int i = 0; i < threadConfigs.size(); i++) {
        qDebug("%s %d %d %d", threadConfigs[i].type.c_str(), threadConfigs[i].deviceNum, threadConfigs[i].typeNum, threadConfigs[i].flipCam);
    }
}

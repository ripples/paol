#include "commandlinethread.h"

CommandLineThread::CommandLineThread(int argc, char **argv) {
    // Make sure there were exactly four arguments given to the program
    assert(argc == 4);

    // Initialize ffmpeg command
    ffmpegCommand = "";

    // Set lecture path
    char* semester = argv[1];
    char* course = argv[2];
    lecturePath = buildLecturePath(semester, course);
    assert(lecturePath != "");

    // Set duration
    char* duration = argv[3];
    lectureDuration = atoi(duration);

    // Read thread configuration from setup file
    setThreadConfigs("/home/paol/paol-code/cameraSetup.txt");
    assert(threadConfigs.size() > 0);
    // Create the threads
    createThreadsFromConfigs();

    // Connect stopCapture signal to processing threads
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        connect(this, SIGNAL(stopCapture()), procThreads[i], SLOT(onQuitProcessing()));
    }
}

CommandLineThread::~CommandLineThread()
{
    // Release the memory used the by the processing threads
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        delete procThreads[i];
    }
    procThreads.clear();
}

void CommandLineThread::run() {
    qDebug("Starting main thread");

    // Print debug information
    for(unsigned int i = 0; i < threadConfigs.size(); i++)
        qDebug("%s %d %d %d", threadConfigs[i].type.c_str(), threadConfigs[i].deviceNum, threadConfigs[i].typeNum, threadConfigs[i].flipCam);
    qDebug("%s", ffmpegCommand.c_str());

    // Start capturing
    system(ffmpegCommand.c_str());
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        procThreads[i]->start();
    }

    // Wait for the duration of the lecture
    sleep(lectureDuration);

    // Signal threads to finish
    system("pkill ffmpeg");
    emit stopCapture();
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        procThreads[i]->wait();
    }

    // Let the main application know that this thread finished
    emit finished();
}

string CommandLineThread::buildLecturePath(string semester, string course) {
    // Get and format course time
    time_t rawTime;
    time(&rawTime);
    char formatDateBuffer[80];
    struct tm * localTime;
    localTime = localtime(&rawTime);
    strftime(formatDateBuffer,80,"%m-%d-%Y--%H-%M-%S",localTime);

    return "/home/paol/recordings/readyToUpload/" + semester + "/" + course + "/" + formatDateBuffer;
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
    int videoDeviceNum = -1;
    bool flipVideo;
    int audioNum = -1;
    for(unsigned int i = 0; i < threadConfigs.size(); i++) {
        ProcThreadConfig c = threadConfigs[i];
        // Switch based on the configuration type
        if(c.type == "Whiteboard") {
            paolProcess* proc = new WhiteboardProcess(c.deviceNum, c.typeNum, c.flipCam, lecturePath);
            procThreads.push_back(proc);
        }
        else if(c.type == "VGA2USB") {
            paolProcess* proc = new VGAProcess(c.deviceNum, c.typeNum, c.flipCam, lecturePath);
            procThreads.push_back(proc);
        }
        else if(c.type == "Video") {
            videoDeviceNum = c.deviceNum;
            flipVideo = c.flipCam;
        }
        else if(c.type == "Audio") {
            audioNum = c.deviceNum;
        }
        else {
            // We found a wrong type, so throw an exception
            throw exception();
        }
    }
    // Make sure the video and audio device numbers were set by the configs
    if(videoDeviceNum == -1 || audioNum == -1) {
        throw exception();
    }

    // Set the command for running ffmpeg
    stringstream ss;
    ss << "/home/paol/paol-code/scripts/capture/videoCapturePortable ";
    ss << videoDeviceNum << " " << audioNum << " " << (int)flipVideo << " ";
    ss << lecturePath << "/video.mp4";
    ffmpegCommand = ss.str();
}

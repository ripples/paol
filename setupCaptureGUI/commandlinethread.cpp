#include "commandlinethread.h"

CommandLineThread::CommandLineThread(int argc, char **argv) {
    /// Make sure there were exactly four arguments given to the program
    assert(argc == 4);

    /// Set course information
    // Set start time of lecture
    time(&startTime);
    // Set semester and course name from argv
    semester = string(argv[1]);
    course = string(argv[2]);
    // Set duration from argv
    lectureDuration = atoi(argv[3]);
    // Set lecture path
    lecturePath = buildLecturePath(semester, course, startTime);
    assert(lecturePath != "");

    // Read thread configuration from setup file
    setThreadConfigs("/home/paol/paol-code/cameraSetup.txt");
}

CommandLineThread::~CommandLineThread()
{
    delete ffmpegProcess;
    // Close the FFmpeg log file if needed
    if(ffmpegLog != NULL) {
        fclose(ffmpegLog);
    }
    // Release the memory used the by the processing threads
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        delete procThreads[i];
    }
    procThreads.clear();
}

void CommandLineThread::run() {
    qDebug("Starting main thread");

    // Make directories
    makeDirectories();
    // Write information file
    writeInfoFile();

    // Initialize handle to FFmpeg log file
    string ffmpegLogPath = lecturePath + "/logs/ffmpeg.log";
    ffmpegLog = fopen(ffmpegLogPath.c_str(), "w");
    assert(ffmpegLog != NULL);

    // Initialize the threads and the FFmpeg QProcess
    createThreadsFromConfigs();

    // Connect stopCapture signal to processing threads and FFmpeg QProcess
    connect(this, SIGNAL(stopCapture()), ffmpegProcess, SLOT(terminate()));
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        connect(this, SIGNAL(stopCapture()), procThreads[i], SLOT(onQuitProcessing()));
    }

    // Connect our FFmpeg output listener
    connect(ffmpegProcess, SIGNAL(readyReadStandardError()), this, SLOT(onFFmpegErrorOutput()));

    // Start capturing from PAOL threads and FFmpeg
    ffmpegProcess->start(ffmpegCommand.c_str());
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        procThreads[i]->start();
    }

    // Wait for the duration of the lecture, then signal threads to finish
    sleep(lectureDuration);
    emit stopCapture();

    // Wait for FFmpeg and processing threads to finish
    ffmpegProcess->waitForFinished();
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        procThreads[i]->wait();
    }

    // Let the main application know that this thread finished
    qDebug("Finishing main thread");
    emit finished();
}

string CommandLineThread::buildLecturePath(string semester, string course, time_t startTime) {
    // Format course start time
    char formatDateBuffer[80];
    struct tm * localTime;
    localTime = localtime(&startTime);
    strftime(formatDateBuffer,80,"%m-%d-%Y--%H-%M-%S",localTime);

    return "/home/paol/recordings/readyToUpload/" + semester + "/" + course + "/" + formatDateBuffer;
}

void CommandLineThread::makeDirectories() {
    // Set the mkdir commands
    string makeLectureDir = "mkdir -p " + lecturePath;
    string makeComputerDir = "mkdir -p " + lecturePath + "/computer";
    string makeWhiteboardDir = "mkdir -p " + lecturePath + "/whiteboard";
    string makeLogDir = "mkdir -p " + lecturePath + "/logs";

    // Execute the mkdir commands
    system(makeLectureDir.c_str());
    system(makeComputerDir.c_str());
    system(makeWhiteboardDir.c_str());
    system(makeLogDir.c_str());
}

void CommandLineThread::setThreadConfigs(string configLocation) {
    // Reset the thread configurations array
    threadConfigs.clear();

    // Open the configuration file
    QString locAsQStr = QString::fromStdString(configLocation);
    QFile configFile(locAsQStr);
    assert(configFile.open(QIODevice::ReadOnly));

    // Initialize counts for how many whiteboards and VGA feeds there are
    whiteboardCount = 0;
    vgaCount = 0;

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
            assert(scanRes == 3);

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
    assert(threadConfigs.size() > 0);
}

void CommandLineThread::createThreadsFromConfigs() {
    // Set parameters for creating the FFmpeg process
    int videoDeviceNum = -1;
    bool flipVideo;
    int audioNum = -1;

    // Go through the thread configurations, creating the paolProcesses
    // and setting the FFmpeg process parameters along the way
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
            // We found a wrong type, so stop the program
            assert(false);
        }
    }

    // Make sure the video and audio device numbers were set by the configs
    assert(videoDeviceNum != -1 && audioNum != -1);

    // Initialize the QProcess for FFmpeg
    ffmpegProcess = new QProcess(this);

    // Set the command for running ffmpeg
    stringstream ss;
    ss << "/home/paol/paol-code/scripts/capture/videoCapturePortable ";
    ss << "/dev/video" << videoDeviceNum << " hw:" << audioNum << " " << (int)flipVideo << " ";
    ss << lecturePath << "/video.mp4";
    ffmpegCommand = ss.str();
    assert(ffmpegCommand != "");
}

void CommandLineThread::writeInfoFile() {
    // Set info file path
    string infoPath = lecturePath + "/INFO";
    // Construct stream to write to info file
    QFile infoFile(QString::fromStdString(infoPath));
    infoFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream infoStream(&infoFile);

    // Format course start time
    char formatDateBuffer[80];
    struct tm * localTime;
    localTime = localtime(&startTime);
    //2014,12,04,12,55,01
    strftime(formatDateBuffer,80,"%Y,%m,%d,%H,%M,%S",localTime);

    // Get host name
    char hostname[256];
    gethostname(hostname, 256);

    // Write the file
    infoStream << "[course]" << endl;
    infoStream << "id: " << course.c_str() << endl;
    infoStream << "term: " << semester.c_str() << endl;
    infoStream << endl;
    infoStream << "[pres]" << endl;
    infoStream << "start: " << formatDateBuffer << endl;
    infoStream << "duration: " << lectureDuration << endl;
    infoStream << "source: " << hostname << endl;
    infoStream << "timestamp: " << startTime << endl;
    infoStream << "whiteboardCount: " << whiteboardCount << endl;
    infoStream << "computerCount: " << vgaCount << endl;

    // Close the file
    infoFile.close();
}

void CommandLineThread::onFFmpegErrorOutput() {
    QByteArray output = ffmpegProcess->readAllStandardError();
    fprintf(ffmpegLog, output.data());
}

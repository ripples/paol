#include "commandlinethread.h"
#include <iostream>

CommandLineThread::CommandLineThread(int argc, char **argv) {
    /// Make sure there were exactly four arguments given to the program
    assert(argc == 4);

    //set up path to file locations
    int loc=QDir::currentPath().toStdString().find("/paol-code/");
    std::string pathTemp=QDir::currentPath().toStdString();
    codePath=pathTemp.substr(0,loc);

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
    string tempPath=codePath+"/paol-code/cameraSetup.txt";
    createUSBSetupFile(tempPath.c_str());
    string tempPath2=codePath+"/paol-code/USBSetup.txt";

    setThreadConfigs(tempPath2.c_str(), tempPath.c_str());
}

CommandLineThread::~CommandLineThread()
{

    delete ffmpegProcess;
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

    // Initialize the threads and the FFmpeg QProcess
    createThreadsFromConfigs();

    // Connect stopCapture signal to processing threads
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        connect(this, SIGNAL(stopCapture()), procThreads[i], SLOT(onQuitProcessing()));
    }

    // Start capturing from PAOL threads and FFmpeg
    ffmpegProcess->start(ffmpegCommand.c_str());//tried moving this after cameras start since video working while cameras failing
    sleep(5);
    for(unsigned int i = 0; i < procThreads.size(); i++) {
        procThreads[i]->start();
    }
;

    // Wait for the duration of the lecture, then signal threads to finish
    sleep(lectureDuration);
    emit stopCapture();
    // Stop FFmpeg
    //ffmpegProcess->write("q");
    //Kill gst process through system - send SIGINT
    system("ps -ef | awk '/[g]st-launch-1.0/{print $2}' | xargs kill -INT");
    sleep(3);
    ffmpegProcess->closeWriteChannel();

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

    return codePath+"/recordings/readyToUpload/" + semester + "/" + course + "/" + formatDateBuffer;
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

void CommandLineThread::setThreadConfigs(string configLocation, string deviceLocation) {
    // Reset the thread configurations array
    threadConfigs.clear();

    // Open the configuration file
    QString locAsQStr = QString::fromStdString(configLocation);
    QFile configFile(locAsQStr);
    assert(configFile.open(QIODevice::ReadOnly));

    QString locDevice = QString::fromStdString(deviceLocation);
    QFile configFile2(locDevice);
    assert(configFile2.open(QIODevice::ReadOnly));

    // Initialize counts for how many whiteboards and VGA feeds there are
    whiteboardCount = 0;
    vgaCount = 0;

    // Read lines from the config file
    QTextStream in(&configFile);
    QTextStream in2(&configFile2);

    while(!in.atEnd() && !in2.atEnd()) {
        QString line = in.readLine();
        QString line2 = in2.readLine();

        // Only parse non-empty lines

        if(line.length() > 0 && line2.length() > 0) {

            // Initialize fields to scan into
            char deviceUSB[100];
            int deviceNum;
            int flipCam;
            char type[16];

            int scanRes = sscanf(line.toStdString().data(), "%s %d %s", deviceUSB, &flipCam, type);
            //read in the extra device information
            int scanRes2 = sscanf(line2.toStdString().data(), "%d %d %s", &deviceNum, &flipCam, type);


            // Make sure exactly three items were found on the current line
            assert(scanRes == 3);

            // Set thread configuration struct for the current line
            ProcThreadConfig p;
            p.deviceUSB = string(deviceUSB);
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

////////////////////////////////////////////////////////////////////////////////////////
void CommandLineThread::createThreadsFromConfigs() {
    // Set parameters for creating the FFmpeg process
    int videoDeviceNum = -1;
    bool flipVideo;
    int audioNum = -1;
    stringstream videoDeviceNumStr, audioNumStr;

    // Go through the thread configurations, creating the paolProcesses
    // and setting the FFmpeg process parameters along the way
    for(unsigned int i = 0; i < threadConfigs.size(); i++) {
        ProcThreadConfig c = threadConfigs[i];
        // Switch based on the configuration type
        if(c.type == "Whiteboard") {
            paolProcess* proc = new WhiteboardProcess(c.deviceUSB, c.deviceNum, c.typeNum, c.flipCam, lecturePath);
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
    // Set the output of the FFmpeg process to a log file
    string ffmpegLogPath = lecturePath + "/logs/ffmpeg.log";
    ffmpegProcess->setStandardErrorFile(QString::fromStdString(ffmpegLogPath));

    videoDeviceNumStr << videoDeviceNum;

    if(audioNum != 0){
        audioNumStr << audioNum;
    } else {
        //If the audio is set to 0, change it to standard 1 to capture audio card 1
        audioNum = 1;
        audioNumStr << audioNum;
    }

    if(flipVideo){
        //if camera is upside down then flip video in capture

        ffmpegCommand = "gst-launch-1.0 -e v4l2src device=/dev/video"+videoDeviceNumStr.str()+
                " \\ ! video/x-h264,width=800, height=448, framerate=24/1 ! decodebin ! videoflip method=2 ! queue ! tee name=myvid \\"+
                " ! queue ! xvimagesink sync=false \\"+
                " myvid. ! mux.video_0 \\"+
                " alsasrc device=plughw:"+audioNumStr.str()+" ! audio/x-raw,rate=44100,channels=2,depth=16 ! audioconvert "+
                " ! lamemp3enc ! queue ! mux.audio_0 \\"+
                " avimux name=mux ! filesink location="+lecturePath+"/video.mp4";
    } else {
        //set normal capture for right side up video

        ffmpegCommand = "gst-launch-1.0 -e v4l2src device=/dev/video"+videoDeviceNumStr.str()+
                " \\ ! video/x-h264,width=800, height=448, framerate=24/1 ! tee name=myvid \\"+
                " ! queue ! decodebin ! xvimagesink sync=false \\"+
                " myvid. ! queue ! mux.video_0 \\"+
                " alsasrc device=plughw:"+audioNumStr.str()+" ! audio/x-raw,rate=44100,channels=2,depth=16 ! audioconvert "+
                " ! lamemp3enc ! queue ! mux.audio_0 \\"+
                " avimux name=mux ! filesink location="+lecturePath+"/video.mp4";
    }

    assert(ffmpegCommand != "");
}
void CommandLineThread::createUSBSetupFile(string cameraFile){
    //vector for video and USB
    vector< vector<string> >usbVideo;
    vector<string>temp;

    //file to read from terminal
    FILE *ptr;
    int bufSize = 512;
    char *buf = new char[bufSize];
    string setupInfo = "";
    std::string outFileName;
    ptr = popen("v4l2-ctl --list-devices", "r");
    int level = 0;
    QString locAsQStr = QString::fromStdString(cameraFile);
    QFile configFile(locAsQStr);
    assert(configFile.open(QIODevice::ReadOnly));
    QTextStream in(&configFile);
    //loop through the usb and videos and add them to the 2d vector
     while(fgets(buf, bufSize, ptr)){
         if(level == 0){
             outFileName = std::string(buf);
             std::size_t found = outFileName.find("usb");
             int found3 = outFileName.length() - found;
             std::string substring = outFileName.substr(found, (found3 - 3));
             temp.push_back(substring);
             level++;
         }
         else if(level == 1){
             outFileName = std::string(buf);
             std::size_t found = outFileName.find("o");
             std::string substring = outFileName.substr(found + 1, 1);
             temp.push_back(substring);
             usbVideo.push_back(temp);
             level++;
         }
         else if(level == 2){
            temp.clear();
            level = 0;
         }
      }
     //loop through the setup file to compare values to USB vector
     while (!in.atEnd()){
         QString line = in.readLine();
         int deviceNum;
         int flipCam;
         char type[16];
         sscanf(line.toStdString().data(), "%d %d %s", &deviceNum, &flipCam, type);
         //device num
         std::string myDevice;
         std::stringstream out;
         out << deviceNum;
         myDevice = out.str();
         //flipped
         std::string myFlip;
         std::stringstream pout;
         pout << flipCam;
         myFlip = pout.str();
         std::cout << usbVideo.size() << endl;
         for(int j = 0; j<usbVideo.size(); j++){
            if(((usbVideo[j][1]).compare(myDevice)) == 0){
                setupInfo = setupInfo + usbVideo[j][0] + " " + myFlip + " " + type + "\n";
            }
         }
    }
    //create USB setup file
     const char *path =(codePath+"/paol-code/USBSetup.txt").c_str();
     ofstream file(path);
     file << setupInfo;
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

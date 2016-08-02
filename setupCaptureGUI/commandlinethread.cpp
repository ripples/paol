#include "commandlinethread.h"

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
    setThreadConfigs(tempPath.c_str());
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
            char deviceUSB[100];
            int deviceNum;
            int flipCam;
            char type[16];
            int scanRes = sscanf(line.toStdString().data(), "%s %d %d %s", deviceUSB, &deviceNum, &flipCam, type);

            // Make sure exactly three items were found on the current line
            assert(scanRes == 4);

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

void CommandLineThread::createThreadsFromConfigs() {
    // Set parameters for creating the FFmpeg process
    int videoDeviceNum = -1;
    bool flipVideo;
    int audioNum = -1;
    int audioNumFix;
    int bufSize = 512;
    char *buf = new char[bufSize];
    FILE *ptr;
    string outFileName;
    string audioSet, cameraAudio, cameraAudioStr;
    string videoDeviceNumStr,audioNumStr;
    stringstream out, audioNumFixStr;

    // Go through the thread configurations, creating the paolProcesses
    // and setting the FFmpeg process parameters along the way
    for(unsigned int i = 0; i < threadConfigs.size(); i++) {
        ProcThreadConfig c = threadConfigs[i];
        // Switch based on the configuration type
        if(c.type == "Whiteboard") {
            paolProcess* proc = new WhiteboardProcess(c.deviceUSB, c.deviceNum, c.typeNum, c.flipCam, lecturePath,false);
            procThreads.push_back(proc);
        }
        else if(c.type == "VGA2USB") {
            paolProcess* proc = new VGAProcess(c.deviceNum, c.typeNum, c.flipCam, lecturePath,false);
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

    //Set video number
    out << videoDeviceNum;
    videoDeviceNumStr = out.str();
    out.str(string());

    //Set audio number
    out << audioNum;

    //Set the string if the pulse audio needs to be fixed
    audioNumFix = audioNum - 1;

    if(audioNumFix != -1)
        audioNumFixStr << audioNumFix;

    cameraAudio = "v4l2-ctl --list-device | grep -A1 HD | grep -o 'video0[^\n]*' | xargs";

    if ((ptr = popen(cameraAudio.c_str(), "r")) != NULL){
            while(fgets(buf, bufSize, ptr)){
                outFileName += buf;
                cameraAudioStr = outFileName;
            }
            fclose(ptr);
        }

    //Set pulsesrc Audio for video record
    if(cameraAudioStr == ""){
        //If the string is empty, the first C920 camera isn't assigned to video0,
        //so this need to be done to keep the pulsesrc audio working properly
        if(out.str() == "1"){
            audioSet = "pactl list short sources | cut -f2 | grep C920.analog";
        } else {
            audioSet = "pactl list short sources | cut -f2 | grep C920_"+audioNumFixStr.str()+".analog";
        }
    } else {
        //Set pulsesrc Audio for video record
        if(out.str() == "0"){
            //If the first C920 camera is assigned to /dev/video0
            audioSet = "pactl list short sources | cut -f2 | grep C920.analog";
        } else {
            //If the audio is assigned to any other C920 camera
            audioSet = "pactl list short sources | cut -f2 | grep C920_"+out.str()+".analog";
        }
    }

    if ((ptr = popen(audioSet.c_str(), "r")) != NULL){
        while(fgets(buf, bufSize, ptr)){
            outFileName += buf;
            audioNumStr = outFileName;
        }
        fclose(ptr);
    }

     qDebug() << audioNumStr.c_str();

    //new method of calling ffmpeg directly from the code without a script
    if(flipVideo){
        //if camera is upside down then flip video in capture
        ffmpegCommand = "gst-launch-1.0 -e v4l2src device=/dev/video"+videoDeviceNumStr+
                " \\ ! video/x-h264,width=320, height=240, framerate=24/1 ! h264parse ! avdec_h264 ! videoflip method=2 ! tee name=myvid \\"+
                " myvid. ! queue ! x264enc ! mux.video_0 \\"+
                " pulsesrc device="+audioNumStr+" ! audio/x-raw,rate=44100,channels=2,depth=16 ! audioconvert "+
                " ! voaacenc ! aacparse ! queue ! mux.audio_0 \\"+
                " mp4mux name=mux ! filesink location="+lecturePath+"/videoLarge.mp4";
    } else {
        //set normal capture for right side up video

        ffmpegCommand = "gst-launch-1.0 -e v4l2src device=/dev/video"+videoDeviceNumStr+
                " \\ ! video/x-h264,width=320, height=240, framerate=24/1 ! h264parse ! tee name=myvid \\"+
                " myvid. ! queue ! mux.video_0 \\"+
                " pulsesrc device="+audioNumStr+" ! audio/x-raw,rate=44100,channels=2,depth=16 ! audioconvert "+
                " ! voaacenc ! aacparse ! queue ! mux.audio_0 \\"+
                " mp4mux name=mux ! filesink location="+lecturePath+"/videoLarge.mp4";

        //ORIGINAL CODE - TESTING

        /*ffmpegCommand = "ffmpeg -s 853x480 -f video4linux2 -i /dev/video"+videoDeviceNumStr.str()+
                " -f alsa -ac 2 -i hw:"+audioNumStr.str()+" -acodec libfdk_aac -b:a 128k "+
                "-vcodec libx264 -preset ultrafast -b:v 260k clear-profile:v baseline -level 3.0 "+
                "-pix_fmt yuv420p -flags +aic+mv4 -threads 0 -r 30 video.mp4 ";*/

        //ORIGINAL CODE - TESTING

    }

    /*old setup for running ffmpeg using script
    // Set the command for running ffmpeg
    stringstream ss;
    ss << codePath << "/paol-code/scripts/capture/videoCapture ";
    ss << "/dev/video" << videoDeviceNum << " hw:" << audioNum << " " << (int)flipVideo << " ";
    ss << lectureDuration << " " << lecturePath << "/video.mp4 ";
    ffmpegCommand = ss.str();
    */

    assert(ffmpegCommand != "");
    /*old that kept failing
    stringstream ss;
    ss << codePath << "/paol-code/scripts/capture/videoCapturePortable ";
    ss << "/dev/video" << videoDeviceNum << " hw:" << audioNum << " " << (int)flipVideo << " ";
    ss << lecturePath << "/video.mp4 ";
    ffmpegCommand = ss.str();
    assert(ffmpegCommand != "");
    */
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

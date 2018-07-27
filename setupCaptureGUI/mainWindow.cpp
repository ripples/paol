#include "mainWindow.h"
#include "ui_mainWindow.h"
#include "clickable_label.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
        ui->setupUi(this);
        continueToCapture = true;
        //Disable all QWidgets that should not be visible on launch (Setup, Whiteboard Edges, Capture)
        ui->setupMenuWidget->hide();
        ui->whiteboardCornersWidget->hide();
        ui->lectureDetailsWidget->hide();
        ui->captureLectureWidget->hide();
        ui->wbc_label->setScaledContents(true);
        videoCapture = false;
        position = 0;
        QTimer *qTimer=new QTimer(this);
        connect(qTimer,SIGNAL(timeout()),this,SLOT(launch_System()));
        connect(ui->wbc_label, SIGNAL(Mouse_Pressed()), this, SLOT(Mouse_Pressed()));
        qTimer->start(1);

        ffmpegProcess = new QProcess(this);
        //qDebug()<< QDir::currentPath();
        //qDebug()<< QDir::currentPath().toStdString().find("/paol-code/");
        int loc=QDir::currentPath().toStdString().find("/paol-code/");
        std::string pathTemp=QDir::currentPath().toStdString();
        codePath=pathTemp.substr(0,loc);
        //qDebug()<< QString::fromUtf8(codePath.c_str());
        videoUSB();
        audioCode();
    }

MainWindow::~MainWindow(){
    delete ffmpegProcess;
    delete ui;
}

///////////////////////////////////////////////////////////////
///           Runner function for Capture Sys              ///
/////////////////////////////////////////////////////////////

void MainWindow::launch_System(){
    if(ui->setupMenuWidget->isVisible()){
        for(int i=0; i < camCount; i++){
            Mat display;
            recordingCams[i] >> frame;
            if(reverseChecks[i]->isChecked())
                flip(frame, frame, -1);
            if(frame.channels() >= 3){ //If statement included in case a camera is connected, but not properly capturing
                cvtColor(frame,display,CV_BGR2RGB);
                QImage img = QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
                imLabels[i]->setPixmap(QPixmap::fromImage(img));
            }
        }
    }

    else if(ui->lectureDetailsWidget->isVisible()){
        if(ui->lecDet_courses->currentText() == "Other"){
            ui->newCourseLabel->show();
            ui->new_course_textbox->show();
        }
        else{
            ui->newCourseLabel->hide();
            ui->new_course_textbox->hide();
        }
    }
    else if(ui->captureLectureWidget->isVisible() && videoCapture == true){
        for(int i = 0; i < dev.length(); i++){
            dev[i]->start();
            }
      videoCapture = false;
    }
    else if(ui->captureLectureWidget->isVisible() && videoCapture == false){
        timer();
    }
}

///////////////////////////////////////////////////////////////
///               GUI Manipulating Functions               ///
/////////////////////////////////////////////////////////////

void MainWindow::populateSetupWindow(){
    for (int i = 0; i < camCount; i++) {
        QGridLayout *const newLayout = new QGridLayout;
        setupLayouts.push_back(newLayout);

        QLabel *const label = new QLabel(QString("CAM %1").arg(i, 2, 10, QLatin1Char('0')));
        label->setScaledContents(true);
        imLabels.push_back(label);

        QComboBox *const comboBox = new QComboBox;
        comboBox->addItem("Whiteboard");
        comboBox->addItem("VGA2USB");
        comboBox->addItem("Video");
        comboBox->addItem("Disabled");
        optionBoxes.push_back(comboBox);

        QCheckBox *const checkBox = new QCheckBox("Check this box if image is upside-down",this);
        reverseChecks.push_back(checkBox);

        QRadioButton *const radioButton = new QRadioButton("Should this device capture the audio",this);
        audioRecord.push_back(radioButton);

        newLayout->addWidget(label,0,0);
        newLayout->addWidget(comboBox);
        newLayout->addWidget(checkBox);
        newLayout->addWidget(radioButton);

        ui->setupGridLayout->addLayout(newLayout,((i-1)+1) / 3, ((i-1)+1) % 3);

        VideoCapture cap(i);

        cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
        recordingCams.push_back(cap);
    }
}

void MainWindow::populateCaptureWindow(){
    int position = 0;
    int vgaCount=0;
    int wbCount=0;
    string USB;

    qDebug() << "Adding to Capture Windows";
    for(int i = 0; i < optionBoxes.length(); i++){
        //qDebug() << "On Number: " << i;
        if( strcmp(optionBoxes[i]->currentText().toUtf8().data(), "Video") != 0 && strcmp(optionBoxes[i]->currentText().toUtf8().data(), "Disabled") != 0){
            //qDebug() << "Looking at " << optionBoxes[i]->currentText();
            QGridLayout *const newLayout = new QGridLayout;
            capLayouts.push_back(newLayout);

            QLabel *imgLabel = new QLabel(QString("CAM %1").arg(i, 2, 10, QLatin1Char('0')));
            QLabel *paolLabel = new QLabel(QString("PAOL %1").arg(i, 2, 10, QLatin1Char('0')));

            imgLabel->setScaledContents(true);
            paolLabel->setScaledContents(true);

            camLabels.push_back(imgLabel);
            paolLabels.push_back(paolLabel);

            newLayout->addWidget(imgLabel,1,0);
            newLayout->addWidget(paolLabel,2,0);

            paolProcess* thread;
            //qDebug() << optionBoxes[i]->currentText();

            if(optionBoxes[i]->currentText() == "VGA2USB"){
                qDebug() << "Adding USB from Camera Num:" << i;
                thread = new VGAProcess(i, vgaCount, false, processLocation,true);
                vgaCount++;
            }
            else if(optionBoxes[i]->currentText() == "Whiteboard"){
                qDebug() << "Adding Whiteboard from Camera Num:" << i;

                std::string myCompare;
                std::stringstream output;
                output << i;
                myCompare = output.str();

                for(unsigned int n=0; n<usbVideo.size(); n++){
                    if(usbVideo[n][1].compare(myCompare) == 0){
                        USB = usbVideo[n][0];
                        break;
                    }
                }
                if(reverseChecks[i]->isChecked())
                    thread = new WhiteboardProcess(USB, i, wbCount, true, processLocation,true);
                else
                    thread = new WhiteboardProcess(USB, i, wbCount, false, processLocation,true);
                wbCount++;
            }

            dev.push_back(thread);

            // Associate the processing thread with the proper views in the capture window
            //qDebug() << "Attaching Label to thread at position " << position;
            threadToUIMap[thread] = position;
            position++;

            // Initialize the slots for updating the UI and stopping the processing threads
            connect(thread, SIGNAL(capturedImage(Mat,paolProcess*)), this, SLOT(onImageCaptured(Mat,paolProcess*)));
            connect(thread, SIGNAL(savedImage(Mat,paolProcess*)), this, SLOT(onImageSaved(Mat,paolProcess*)));
            connect(this, SIGNAL(quitProcessing()), thread, SLOT(onQuitProcessing()));
            //qDebug() << "Row: " << 4/captureDevices << ", Column: " << (i % captureDevices);
            ui->lectureCaptureGrid->addLayout(newLayout, 4 / captureDevices, i);
        }
    }
}

void MainWindow::createSetupNavigationBar(){
    QLabel *const first = new QLabel(QString("1. Camera Setup"));
    QLabel *const second = new QLabel(QString("2. Whiteboard Corners"));
    QLabel *const third = new QLabel(QString("3. Lecture Details"));
    QLabel *const fourth = new QLabel(QString("4. Lecture Capture"));

    locationText.push_back(first);
    locationText.push_back(second);
    locationText.push_back(third);
    locationText.push_back(fourth);

/*
    if(continueToCapture == true){
        for(int i = 0; i < 4; i++){
            ui->locationBar->addWidget(locationText[i],0,0);
        }
    }
    else{
        for(int i = 0; i < 2; i++){
            ui->locationBar->addWidget(locationText[i],0,0);
        }
    }*/
}


///////////////////////////////////////////////////////////////
///                     Utilities                          ///
/////////////////////////////////////////////////////////////
//Find the amount of cameras currently connected to the machine
void MainWindow::countCameras(){
    int bufSize = 512;
    char *buf = new char[bufSize];
    FILE *ptr;
    string outFileName;
    camCount = 0;
    int temp;

    //Obtain amount of cameras connected to this device
    if ((ptr = popen("ls /dev/video*", "r")) != NULL){
        while(fgets(buf, bufSize, ptr)){
            outFileName = string(buf);
            outFileName = outFileName.substr(10,outFileName.find('\n'));
            outFileName = outFileName.substr(0,outFileName.find('\n'));

            stringstream(outFileName) >> temp;
            camCount++;
        }
        fclose(ptr);
    }
    qDebug() << "Number of Cameras plugged in: " << camCount;
}

void MainWindow::videoUSB(){
    FILE *ptr;
    int bufSize = 512;
    char *buf = new char[bufSize];
    std::string outFileName;
    ptr = popen("v4l2-ctl --list-devices", "r");
    int level = 0;
    vector<string>temp;
    //loop through the terminal command to store the USB/video values in vector field
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
            level = 0;
            temp.clear();
         }
      }
}

void MainWindow::audioCode(){
    std::string model = "C920";
    FILE *pointer, *address;
    int Size = 512;
    char *buffer = new char[Size];
    char *buffer2 = new char[Size];
    std::string output;
    std::string out;
    pointer = popen("v4l2-ctl --list-devices", "r");
    address = popen("pactl list short sources | cut -f2 | grep C9 | grep analog", "r");
    int level = 0;
    bool camera;
    vector<string>temp;
    vector<string>device;
    vector<string> local;
    ////////////////////////////////////
     while(fgets(buffer, Size, pointer)){
        if(level == 0){
            output = std::string(buffer);
//            std::size_t found = output.find("HD Pro Webcam "+model);
            std::size_t found = output.find("Logitech Webcam "+model);
            if(found != string::npos){
                camera = true;
                level++;
            }
            else{
                camera = false;
                level++;
            }
        }
        else if(level == 1){
            if(camera == true){
                output = std::string(buffer);
                std::size_t found = output.find("o");
                std::string substring = output.substr(found + 1, 1);
                temp.push_back(substring);
                level++;
            }
            else{
                level++;
            }
         }
        else if(level == 2){
             level = 0;
        }
     }
     ////////////////////////////////////////////////////
     while(fgets(buffer2, Size, address)){
         out = std::string(buffer2);
//         string terminal = "alsa_input.usb-046d_HD_Pro_Webcam_"+model+"_";
//         string terminal = "alsa_input.usb-046d_Logitech_Webcam_"+model+"_";
         string terminal = "alsa_input.usb-046d_HD_Pro_Webcam_"+model+"_";
         alsaString = out.substr(terminal.length(), 8);
         device.push_back(alsaString);
     }
     //////////////////////////////////////////////////////
     if(temp.size() > 1){
         string swap;
         int n = temp.size();
         for (int c = 0 ; c < ( n - 1 ); c++)
           {
             for (int d = 0 ; d < n - c - 1; d++)
             {
               if (temp[d] > temp[d+1])
               {
                 swap = temp[d];
                 temp[d] = temp[d+1];
                 temp[d+1] = swap;
               }
             }
           }
     }
     for(unsigned int p=0; p<temp.size(); p++){
         local.clear();
         local.push_back(temp[p]);
         local.push_back(device[p]);
         audioNumber.push_back(local);
     }
}
//Create string that will be stored to the INFO file
void MainWindow::createInfoFile(){
    string INFOFileText = "";
    stringstream ss;
    int compCount = 0;
    int wbCount = 0;

    //Get time since Epoch
    time_t epoch = time(0);
    ss << epoch;
    INFOFileText = "timestamp: " + ss.str() + "\n";

    ss.str(string()); //Clear out stringstream

    //Get connected Whiterboard cameras and Computer amounts
    for(int i = 0; i < optionBoxes.length(); i++){
        if(optionBoxes[i]->currentText() == "Whiteboard"){
            wbCount++;
        }
        else if(optionBoxes[i]->currentText() == "VGA2USB"){
            compCount++;
        }
    }

    ss << wbCount;
    INFOFileText = INFOFileText + "whiteboardCount: " + ss.str() + "\n";
    ss.str(string()); // Clear out stringstream

    ss << compCount;
    INFOFileText = INFOFileText + "computerCount: " + ss.str() + "\n";

    string outLocation = processLocation + "/INFO";
    const char *path = outLocation.data();
    //Creates .txt file to which outputInfo is placed in
    //qDebug() << outLocation.data();
    std::ofstream file(path);
    file << INFOFileText;
}

void MainWindow::createCameraSetupFile(){
    string setupInfo = "";
    for( int i = 0; i < optionBoxes.size(); i++){
        string device = optionBoxes[i]->currentText().toLatin1().data();
        int isFlipped = 0;

        if(device != "Disabled"){
            if(reverseChecks[i]->isChecked() == true){
                isFlipped = 1;
            }

            stringstream out;

            string num;
            string USB;
            string flipped;

            out << i;
            num = out.str();
            out.str(string());

            out << isFlipped;
            flipped = out.str();

            for(unsigned int i=0; i<usbVideo.size(); i++){
                if(usbVideo[i][1].compare(num) == 0){
                     USB = usbVideo[i][0];
                     break;
                }
            }
            setupInfo = setupInfo + USB + " " + num + " " + flipped + " " + device + "\n";

            if(audioRecord[i]->isChecked() == true){
               string audio;
               for(unsigned int h=0; h<audioNumber.size(); h++){
                   if(audioNumber[h][0].compare(num) == 0){
                        audio = audioNumber[h][1];
                        break;
                   }
               }
                setupInfo = setupInfo + audio + " " + num + " 0 Audio\n";
            }
        }
    }
    //qDebug() << QString::fromUtf8(codePath+"/paol-code/cameraSetup.txt");
    string fullPath=codePath+"/paol-code/cameraSetup.txt";
    const char *path =fullPath.data();//codePath+"/paol-code/cameraSetup.txt").c_str();
    //qDebug(fullPath.c_str());
    //qDebug(setupInfo.c_str());
    ofstream file(path);
    file << setupInfo;
}

//Fills the QComboBox on the Classroom information view with available classes
void MainWindow::populateCourseList(){
    ifstream inputFile((codePath+"/paol-code/courses.txt").c_str());
    string str;
    while(getline(inputFile, str)){
        ui->lecDet_courses->addItem(str.data());
    }
    ui->lecDet_courses->addItem("Other");
}

void MainWindow::createWBCornerTxt(){
    string allCoordinates = "";
    stringstream out;
    string filePath;

    string cornerLocation;
    std::string myCompare;
    std::stringstream output;
    output << corners_currentCam;
    myCompare = output.str();

    //make wb corners correspond to USB instead of Wb number
    for(unsigned int i=0; i<usbVideo.size(); i++){
        if(usbVideo[i][1].compare(myCompare) == 0){
             cornerLocation = usbVideo[i][0];
             break;
        }
    }
    out << cornerLocation;

    filePath = codePath+"/paol-code/wbCorners" + out.str() + ".txt";
    out.str(string());

    for(int i = 0; i < intersectionPoints.length(); i++){
        out << intersectionPoints[i].x;
        allCoordinates += out.str() + ",";
        out.str(string());
        out << intersectionPoints[i].y;
        allCoordinates += out.str();
        allCoordinates += "\n";
        out.str(string());
    }

    const char *path = filePath.data();
    ofstream file(path);
    file << allCoordinates;
}

void MainWindow::createFileDirectory(){
    time_t theTime = time(NULL);
    struct tm *aTime = localtime(&theTime);
    int year = aTime->tm_year + 1900;
    stringstream toString;

    string currentYear;
    toString << year;
    currentYear = toString.str();

    // Clear string stream
    toString.str(string());

    string courseNumber;
    if(ui->lecDet_courses->currentText() != "Other"){
        courseNumber = ui->lecDet_courses->currentText().toLatin1().data();
    }
    else{
        courseNumber = ui->new_course_textbox->text().toLatin1().data();
    }

    //system("mkdir -p /home/paol/recordings/readyToUpload");
    system(("mkdir -p "+codePath+"/recordings/readyToUpload").c_str());
    string firstCmd = "mkdir -p "+codePath+"/recordings/readyToUpload/" + currentYear;
    system(firstCmd.data());

    string secondCmd = "mkdir -p "+codePath+"/recordings/readyToUpload/" + currentYear + "/" + courseNumber;
    system(secondCmd.data());

    string thirdCmd = "mkdir -p "+codePath+"/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + "";
    system(thirdCmd.data());

    char buffer[80];
    time (&theTime);
    aTime = localtime(&theTime);
    strftime(buffer,80,"%m-%d-%Y--%H-%M-%S",aTime);

    string fourthCmd = "mkdir -p "+codePath+"/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + buffer;
    system(fourthCmd.data());

    processLocation = codePath+"/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + buffer;

    string makeComputerDir = "mkdir -p " + processLocation + "/computer";
    string makeWhiteboardDir = "mkdir -p " + processLocation + "/whiteboard";
    string makeLogDir = "mkdir -p " + processLocation + "/logs";

    system(makeComputerDir.c_str());
    system(makeWhiteboardDir.c_str());
    system(makeLogDir.c_str());
}

void MainWindow::navigationBarUpdate(bool direction){
    QFont bold;
    QFont unbold;
    bold.setBold(true);
    unbold.setBold(false);

    if(direction){
        qDebug() << "Going forward";
        qDebug() <<"Position before is: " << position;
        locationText[position]->setFont(bold);

        if(position != 0){
            locationText[position - 1]->setFont(unbold);
        }
        position++;
        qDebug() <<"Position after is: " << position;
    }

    else{
        locationText[position-1]->setFont(unbold);
        locationText[position-2]->setFont(bold);
        position--;
    }
}

void MainWindow::releaseComponents(){

    for(int i = 0; i < dev.length(); i ++){
        delete dev[i];
    }

    for(int j = 0; j < camLabels.length(); j++){
        delete camLabels[j];
        delete paolLabels[j];
        delete capLayouts[j];
    }

    for(int k = 0; k < recordingCams.length(); k++){
        delete imLabels[k];
        delete optionBoxes[k];
        delete reverseChecks[k];
        delete audioRecord[k];
    }

    for(int l = 0; l < locationText.length(); l++){
        delete locationText[l];
    }

    position = 0;

    camLabels.clear();
    paolLabels.clear();
    dev.clear();
    capLayouts.clear();
    recordingCams.clear();
    audioRecord.clear();
    reverseChecks.clear();
    optionBoxes.clear();
    imLabels.clear();
    locationText.clear();
}

void MainWindow::captureVideo(){
    // Find which camera has the Audio box selected
    string audioCamNum;
    string s;
    stringstream out;
    char buff[512];
    FILE *ptr;
    string outFileName;
    string audioSet;
    string cameraAudio, cameraAudioStr;
    stringstream audioNumFixStr;
    int audioNumFix = -1;

    for(int h = 0; h < audioRecord.length(); h++){
        if(audioRecord[h]->isChecked()){
            out << h;
            if(h != 0)
                audioNumFix = h - 1;
        }
    }

    //Set the string if the pulse audio needs to be fixed
    if(audioNumFix != -1)
        audioNumFixStr << audioNumFix;

    cameraAudio = "v4l2-ctl --list-device | grep -A1 C920 | grep -o 'video0[^\\n]*' | xargs";


    if(!(ptr = popen(cameraAudio.c_str(), "r"))){
    }

    while(fgets(buff, sizeof(buff), ptr)!=NULL){
        outFileName += buff;
        cameraAudioStr = outFileName;
    }
    pclose(ptr);

    //Set pulsesrc Audio for video record
    if(cameraAudioStr == ""){
        //If the string is empty, the first C920 camera isn't assigned to video0,
        //so this needs to be done to keep the pulsesrc audio working properly
        if(out.str() == "" || out.str() == "1"){
            audioSet = "pactl list short sources | cut -f2 | grep C920_ | head -1";
        } else {
            audioSet = "pactl list short sources | cut -f2 | grep C920_"+ alsaString;
        }
    } else {
        //Set pulsesrc Audio for video record
        if(out.str() == "" || out.str() == "0"){
            //If the first C920 camera is assigned to /dev/video0
            audioSet = "pactl list short sources | cut -f2 | grep C920_ | head -1";
        } else {
            //If the audio is assigned to any other C920 camera
            audioSet = "pactl list short sources | cut -f2 | grep C920_"+ alsaString;
        }
    }

    if(!(ptr = popen(audioSet.c_str(), "r"))){
    }
    while(fgets(buff, sizeof(buff), ptr)!=NULL){
        outFileName += buff;
        audioCamNum = outFileName;
    }
    pclose(ptr);

    qDebug() << "------------" << audioCamNum.c_str() << "its here";

    for(int i = 0; i < optionBoxes.length(); i++){
        //Find if there is a Video camera selected
        if(optionBoxes[i]->currentText() == "Video"){
            string isChecked = "0";

            if(reverseChecks[i]->isChecked() == true){
                isChecked = "1";
            }

            out.str(string());
            out << i;
            s = out.str();

           // audioCamNum.erase(std::remove(audioCamNum.begin(),audioCamNum.end(),' '),audioCamNum.end());

            std::size_t found = audioCamNum.find("alsa");
            audioCamNum.erase(0,found);
           // qDebug() << audioCamNum.c_str();

            /* old way of calling ffmpeg from script
            vidCaptureString =codePath+"/paol-code/scripts/capture/videoCapturePortable /dev/video" + s + " hw:" + audioCamNum + " "
            + isChecked + " " + processLocation + "/video.mp4 &";
            */


            //call ffmpeg directly
            if(reverseChecks[i]->isChecked()==1){
                //if camera is upside down then flip video in capture
//                vidCaptureString = "gst-launch-1.0 -e v4l2src device=/dev/video"+s+
//                        " ! video/x-h264,width=320, height=240, framerate=24/1 ! h264parse ! avdec_h264 ! videoflip method=2 ! tee name=myvid"+
//                        " myvid. ! queue ! x264enc ! mux.video_0"+
//                        " pulsesrc device="+audioCamNum+" ! audio/x-raw,rate=32000,channels=2,depth=16 ! queue ! audioconvert "+
//                        " ! voaacenc ! queue ! aacparse ! queue ! mux.audio_0"+
//                        " mp4mux name=mux ! filesink location="+processLocation+"/videoLarge.mp4";
                //For flipped video. Actually works
                //vidCaptureString = "gst-launch-1.0 v4l2src -e device=/dev/video" + s +" ! video/x-raw, width=320, height=240, framerate=24/1 ! queue ! videoconvert ! videoflip method=vertical-flip ! x264enc tune=zerolatency ! h264parse ! mux.video_0 pulsesrc device=alsa_input.usb-046d_HD_Pro_Webcam_C920_318C23BF-02.analog-stereo volume=8 ! audio/x-raw, rate=32000, channels=2 ! queue ! voaacenc ! aacparse ! mux.audio_0 mp4mux name=mux ! filesink location=" + processLocation + "/lecture.mp4";
                vidCaptureString = "gst-launch-1.0 v4l2src -e device=/dev/video" + s + " ! video/x-raw, width=320, height=240, framerate=24/1 ! queue ! videoconvert ! videoflip method=vertical-flip ! x264enc tune=zerolatency bitrate=512 ! h264parse ! mux.video_0 pulsesrc device=" + audioCamNum + " volume=8 ! audio/x-raw, rate=32000, channels=2, depth=16 ! queue ! voaacenc ! aacparse ! mux.audio_0 mp4mux name=mux ! filesink location=" + processLocation + "/lecture.mp4";
            } else {
                //set normal capture for right side up video
//                vidCaptureString =  "gst-launch-1.0 -e v4l2src device=/dev/video"+s+
//                        " ! video/x-h264,width=320, height=240, framerate=24/1 ! h264parse ! tee name=myvid"+
//                        " myvid. ! queue ! mux.video_0"+
//                        " pulsesrc device="+audioCamNum+" ! audio/x-raw,rate=32000,channels=2,depth=16 ! audioconvert "+
//                        " ! voaacenc ! aacparse ! queue ! mux.audio_0"+
//                        " mp4mux name=mux ! filesink location="+processLocation+"/videoLarge.mp4";

                //For non-flipped video. Actually works
//                vidCaptureString = "gst-launch-1.0 v4l2src -e device=/dev/video" + s +" ! video/x-h264, width=320, height=240, framerate=24/1 ! h264parse ! queue ! mux. pulsesrc device=alsa_input.usb-046d_HD_Pro_Webcam_C920_318C23BF-02.analog-stereo volume=8 "
//                        + " ! audio/x-raw, rate=32000, channels=2, depth=16 ! voaacenc ! aacparse ! queue ! mux. mp4mux name=mux ! filesink location="+ processLocation +"/lecture.mp4";
             //   vidCaptureString = "gst-launch-1.0 v4l2src -e device=/dev/video"+ s + " ! video/x-raw, width=320, height=240, framerate=24/1 ! queue ! videoconvert ! x264enc tune=zerolatency bitrate=512 ! video/x-h264, profile=constrained-baseline ! h264parse ! mux. pulsesrc device=alsa_input.usb-046d_HD_Pro_Webcam_C920_318C23BF-02.analog-stereo volume=8 ! audio/x-raw, rate=32000, channels=2, depth=16 ! queue ! voaacenc ! aacparse ! mux. mp4mux name=mux ! filesink location=" + processLocation + "/test.mp4";

                vidCaptureString = "gst-launch-1.0 v4l2src -e device=/dev/video"+ s + " ! video/x-raw, width=320, height=240, framerate=24/1 ! queue ! videoconvert ! x264enc tune=zerolatency bitrate=512 ! video/x-h264, profile=constrained-baseline ! h264parse ! mux. pulsesrc device=" + audioCamNum + " volume=8 ! audio/x-raw, rate=32000, channels=2, depth=16 ! queue ! voaacenc ! aacparse ! mux. mp4mux name=mux ! filesink location=" + processLocation + "/lecture.mp4";


                //ORIGINAL CODE <<<<<<<<<<<<<<<<<<<<<<<<<
                /*vidCaptureString = "ffmpeg -s 853x480 -f video4linux2 -i /dev/video"+s+
                        " -f alsa -ac 2 -i hw:"+audioCamNum+" -acodec libfdk_aac -b:a 128k "+
                        "-vcodec libx264 -preset ultrafast -b:v 260k -profile:v baseline -level 3.0 "+
                        "-pix_fmt yuv420p -flags +aic+mv4 -threads 0 -r 30 video.mp4 ";*/
                //ORIGINAL CODE <<<<<<<<<<<<<<<<<<<<<<<<
            }
           // qDebug("--------------%s",vidCaptureString.c_str());
            ffmpegProcess->setStandardErrorFile(QString::fromStdString(processLocation + "/logs/ffmpeg.log"));
            ffmpegProcess->start(vidCaptureString.c_str());
        }
    }
}

void MainWindow::timer(){
     int a = myTimer.elapsed();
     int secondsVal = (a / 1000) % 60;
     int minutesVal = ((a / (1000 *60)) % 60);
     int hoursVal = ((a / (1000*60*60*60)) % 24);
     captureSecondsElapsed = (a / 1000);
     string minutes;
     string hours;
     string seconds;

     if(secondsVal < 10){
         stringstream ss;
         ss << secondsVal;
         seconds = "0" + ss.str();
     }else{
         stringstream ss;
         ss << secondsVal;
         seconds = ss.str();
     }

     if(minutesVal != 0){
         stringstream mm;
         mm << minutesVal;
         minutes = mm.str();
     }else{
         minutes = "00";
     }

     if(hoursVal != 0){
         stringstream hh;
         hh << hoursVal;
         hours = hh.str();
     } else{
         hours = "00";
     }

     string final = "Time Elapsed: " + hours + ":" + minutes + ":" + seconds;
     ui->captureLecture_Timer->setText(QString::fromStdString(final));

     // Check if FFmpeg process is running
     if(ffmpegProcess->state() == QProcess::Running) {
         ui->captureLecture_Video_Status->setText("Video Status: Running");
     }
     else {
         ui->captureLecture_Video_Status->setText("Video Status: NOT RUNNING");
     }
 }

//////////////////////////////////////////////////////////////
///                    Signal handling                    ///
////////////////////////////////////////////////////////////

void MainWindow::onImageCaptured(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    //qDebug("Capture: Send captured image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
    int displayNum = threadToUIMap[threadAddr];
    displayMat(image, *camLabels[displayNum]);
}

void MainWindow::onImageSaved(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    //qDebug("Saved: Send saved image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
    int displayNum = threadToUIMap[threadAddr];
    //qDebug() << displayNum;
    displayMat(image, *paolLabels[displayNum]);
}


//////////////////////////////////////////////////////////////
///                    Image displaying                   ///
////////////////////////////////////////////////////////////

QImage MainWindow::convertMatToQImage(const Mat& mat){
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mat,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888)
            .copy();
    return img;
}

void MainWindow::displayMat(const Mat& mat, QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMatToQImage(mat);
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}


//////////////////////////////////////////////////////////////
///  Button Functions used to transition between windows  ///
////////////////////////////////////////////////////////////

/// MAIN MENU BUTTONS
void MainWindow::on_mainMenu_Full_Run_clicked(){
    countCameras();
    populateSetupWindow();
    createSetupNavigationBar();
    navigationBarUpdate(true);
    ui->mainMenuWidget->hide();
    ui->setupMenuWidget->show();
}

void MainWindow::on_mainMenu_Setup_Cameras_clicked(){
    continueToCapture = false;
    on_mainMenu_Full_Run_clicked();
}

void MainWindow::on_mainMenu_Upload_Lectures_clicked(){
  //  string uploadScript=codePath+"/paol-code/scripts/upload/uploadAll.sh "+codePath;
    string uploadScript="/home/paol/paol-code/scripts/upload/uploadAll.sh";
    qDebug("%s",uploadScript.c_str());
//    QProcess *runUpload=new QProcess(this);
//    runUpload->start(uploadScript.c_str());

//    //Check if upload is still running
//    //This way the GUI freezes
//    qDebug() << "Uploading files. GUI blocked. Please wait.";
//    runUpload->waitForFinished(-1);

//    if(runUpload->exitCode() !=0){
//        qDebug() << "Error";
//    }
//    else{
//        qDebug() << "Upload is Done";
//    }
    string dump;
    int bufSize = 512;
    char *buf = new char[bufSize];
    FILE *ptr;
    if ((ptr = popen(uploadScript.c_str(), "r")) != NULL){
            while(fgets(buf, bufSize, ptr)){
                dump = std::string(buf);
            }
            fclose(ptr);
        }

}

/// SETUP WINDOW BUTTONS
void MainWindow::on_setupContinueButton_clicked(){
    ui->setupMenuWidget->hide();
    corners_currentCam = 0;
    clicked = 0;
    tracker = 0;
    whiteboards = 0;
    captureDevices = 0;

    for(int i = 0; i < optionBoxes.length(); i++){
        if(optionBoxes[i]->currentText() == "Whiteboard"){
            whiteboards += 1;
        }
    }

    for(int i = 0; i < optionBoxes.length(); i++){
        if(optionBoxes[i]->currentText() == "Whiteboard" || optionBoxes[i]->currentText() == "VGA2USB"){
            captureDevices += 1;
        }
    }
    qDebug() << captureDevices;
    navigationBarUpdate(true);
    if(whiteboards != 0){
        while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
            corners_currentCam += 1;
        }

        Mat display;
        recordingCams[corners_currentCam] >> frame;
        if(reverseChecks[corners_currentCam]->isChecked())
            flip(frame, frame, -1);
        cvtColor(frame,display,CV_BGR2RGB);
        corners_Clone = display.clone();
        QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
        ui->wbc_label->setPixmap(QPixmap::fromImage(img));

        ui->whiteboardCornersWidget->show();
    }
    else{
        if(continueToCapture == false){
            ui->mainMenuWidget->show();
            continueToCapture = true;
        }
        else{
            populateCourseList();
            ui->lectureDetailsWidget->show();
        }
    }
}

void MainWindow::on_setupReturnButton_clicked(){
    ui->setupMenuWidget->hide();
    releaseComponents();
    // Stop FFmpeg
    //ffmpegProcess->write("q");
    //system("ps -ef | awk '/[g]st-launch-1.0/{print $2}' | xargs kill -INT");
    ffmpegProcess->closeWriteChannel();
    ffmpegProcess->waitForFinished();
    ui->mainMenuWidget->show();
}


/// WHITEBOARD CORNERS WINDOW BUTTONS
void MainWindow::on_WBC_PrevWB_clicked(){
    intersectionPoints.clear();
    clickedCorners.clear();
    linePoints.clear();
    lineSlopes.clear();
    clicked = 0;

    tracker--;
    if(tracker < 0){
        tracker = whiteboards - 1;
    }

    corners_currentCam -= 1;
    if(corners_currentCam < 0){
        corners_currentCam = camCount - 1;
    }

    while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
        corners_currentCam -= 1;
    }

    place_image();
}

void MainWindow::on_WBC_NextWB_clicked(){
    intersectionPoints.clear();
    clickedCorners.clear();
    linePoints.clear();
    lineSlopes.clear();
    clicked = 0;

    tracker++;
    if(tracker > whiteboards - 1){
        tracker = 0;
    }

    corners_currentCam += 1;
    if(corners_currentCam >= camCount){
        corners_currentCam = 0;
    }

    while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
        corners_currentCam += 1;

        if(corners_currentCam >= camCount){
            corners_currentCam = 0;
        }
    }

    place_image();
}

void MainWindow::on_WBC_Clear_clicked(){
    clickedCorners.clear();
    linePoints.clear();
    lineSlopes.clear();
    intersectionPoints.clear();
    clicked = 0;
    place_image();
}

void MainWindow::findSlopes(){
    double slope,difx,dify;
    for(int i = 0; i < clickedCorners.length()-1; i = i + 2){
        linePoints.append(Point(clickedCorners[i].x,clickedCorners[i].y));
        linePoints.append(Point(clickedCorners[i+1].x,clickedCorners[i+1].y));
        difx=clickedCorners[i].x-clickedCorners[i+1].x;
        dify=clickedCorners[i].y-clickedCorners[i+1].y;
        if(difx==0)
            slope=1000;
        else
            slope=dify/difx;
        if(slope==0)
            slope=.0001;
        lineSlopes.append(slope);
    }
}

void MainWindow::findCorners(){
    int s1a,s1b;
    double dif,minDif;

    //find similar slopes
    s1a=0;
    minDif=1000;
    for (int i=1;i<lineSlopes.length();i++){
        dif=abs(abs(lineSlopes[s1a]/lineSlopes[i])-abs(lineSlopes[i]/lineSlopes[s1a]));
        qDebug("dif=%f val=%f",dif,lineSlopes[i]);
        if(dif<minDif){
            minDif=dif;
            s1b=i;
            qDebug("%d",i);
        }
    }
    //get the similar slopes paired in storage since user may not use correct order
    if(s1b!=1){
        swap(lineSlopes[1],lineSlopes[s1b]);
        swap(linePoints[2*1],linePoints[s1b*2]);
        swap(linePoints[2*1+1],linePoints[s1b*2+1]);
    }
    //put me in to see what's happening
    qDebug("pair one %f %f",lineSlopes[0],lineSlopes[1]);
    qDebug("pair two %f %f",lineSlopes[2],lineSlopes[3]);
    for (int i=0;i<linePoints.length();i++){
        qDebug("xold=%d \txnew=%d",clickedCorners[i].x ,linePoints[i].x);
    }
}

void MainWindow::reorderCorners(){
    //reorder so top two points are first
    for(int j=0;j<2;j++){
        for(int i=j+1;i<intersectionPoints.length();i++){
            if(intersectionPoints[i].y<intersectionPoints[j].y)
                swap(intersectionPoints[j],intersectionPoints[i]);
        }
    }

    //put the points in the correct order Top Left, right, Bottom right, left
    if(intersectionPoints[0].x>intersectionPoints[1].x)
        swap(intersectionPoints[0],intersectionPoints[1]);
    if(intersectionPoints[3].x>intersectionPoints[2].x)
        swap(intersectionPoints[2],intersectionPoints[3]);
}

void MainWindow::on_WBC_Save_clicked(){
    if(clicked == 8){
        place_image();

        findSlopes();
        findCorners();

        //find the corners based on paired corners
        intersectionPoints.append(
                    determineIntersection(linePoints[0].x,linePoints[0].y,linePoints[1].x,linePoints[1].y,linePoints[4].x,
                              linePoints[4].y, linePoints[5].x, linePoints[5].y));
        intersectionPoints.append(
                    determineIntersection(linePoints[0].x,linePoints[0].y,linePoints[1].x,linePoints[1].y,linePoints[6].x,
                              linePoints[6].y, linePoints[7].x, linePoints[7].y));
        intersectionPoints.append(
                    determineIntersection(linePoints[2].x,linePoints[2].y,linePoints[3].x,linePoints[3].y,linePoints[4].x,
                              linePoints[4].y, linePoints[5].x, linePoints[5].y));
        intersectionPoints.append(
                    determineIntersection(linePoints[2].x,linePoints[2].y,linePoints[3].x,linePoints[3].y,linePoints[6].x,
                              linePoints[6].y, linePoints[7].x, linePoints[7].y));

        //reorder corners
        reorderCorners();

        for(int j = 0; j < intersectionPoints.length(); j++){
            circle(corners_Clone,intersectionPoints[j], 8, Scalar(255,0,255),8,8,0);
        }
        line(corners_Clone,intersectionPoints[0],intersectionPoints[1], Scalar(255,0,0), 3,8,0);
        line(corners_Clone,intersectionPoints[1],intersectionPoints[2], Scalar(255,0,0), 3,8,0);
        line(corners_Clone,intersectionPoints[2],intersectionPoints[3], Scalar(255,0,0), 3,8,0);
        line(corners_Clone,intersectionPoints[3],intersectionPoints[0], Scalar(255,0,0), 3,8,0);

        createWBCornerTxt();
    }
    clickedCorners.clear();
    linePoints.clear();
    lineSlopes.clear();
    intersectionPoints.clear();
    QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::Mouse_Pressed(){
    clicked += 1;
    //double posX = (1920.0 / ui->wbc_label->width()) * ui->wbc_label->x;
    //double posY = (1080.0 / ui->wbc_label->height()) * ui->wbc_label->y;
    double posX = ( (double)frame.cols / ui->wbc_label->width()) * ui->wbc_label->x;
    double posY = ( (double)frame.rows / ui->wbc_label->height()) * ui->wbc_label->y;
    clickedCorners.append(Point(posX, posY));
    if(clicked % 2 == 0){
        for(int i = 0; i < clickedCorners.length(); i+=2){
            line(corners_Clone, clickedCorners[i],clickedCorners[i+1], Scalar(0,255,255),5,8,0);
        }
    }

    //qDebug() << "Clicked at: X " << posX << ", Y" << posY;
    circle(corners_Clone, Point(posX,posY), 32.0, Scalar( 255, 255, 0 ), 2, 8);
    QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));

}

void MainWindow::place_image(){
    Mat display;
    // Clear out buffer
    for(int i = 0; i < 6; i++){
        recordingCams[corners_currentCam] >> frame;
        if(reverseChecks[corners_currentCam]->isChecked())
            flip(frame, frame, -1);
    }

    cvtColor(frame,display,CV_BGR2RGB);
    corners_Clone = display.clone();
    QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));
}

Point MainWindow::determineIntersection(double Ax, double Ay, double Bx, double By, double Cx, double Cy, double Dx, double Dy){
    double  distAB, theCos, theSin, newX, ABpos ;

    //  Fail if either line is undefined.
    if ( (Ax==Bx && Ay==By) || (Cx==Dx && Cy==Dy) ){
        return Point(0,0);
    }
    //  (1) Translate the system so that point A is on the origin.
    Bx-=Ax; By-=Ay;
    Cx-=Ax; Cy-=Ay;
    Dx-=Ax; Dy-=Ay;

    //  Discover the length of segment A-B.
    distAB=sqrt(Bx*Bx+By*By);

    //  (2) Rotate the system so that point B is on the positive X axis.
    theCos=Bx/distAB;
    theSin=By/distAB;
    newX=Cx*theCos+Cy*theSin;
    Cy  =Cy*theCos-Cx*theSin; Cx=newX;
    newX=Dx*theCos+Dy*theSin;
    Dy  =Dy*theCos-Dx*theSin; Dx=newX;

    //  Fail if the lines are parallel.
    if (Cy==Dy){
        Point(0,0);
    }

    //  (3) Discover the position of the intersection point along line A-B.
    ABpos=Dx+(Cx-Dx)*Dy/(Dy-Cy);

    //  (4) Apply the discovered position to line A-B in the original coordinate system.
    double X =Ax+ABpos*theCos;
    double Y =Ay+ABpos*theSin;

    //  Success.
    return Point(X,Y);
}

void MainWindow::on_WBC_Return_Button_clicked(){
    ui->whiteboardCornersWidget->hide();
    clickedCorners.clear();
    intersectionPoints.clear();
    corners_currentCam = 0;
    navigationBarUpdate(false);
    ui->setupMenuWidget->show();
}

void MainWindow::on_WBC_Continue_Button_clicked(){
    ui->whiteboardCornersWidget->hide();

    clickedCorners.clear();
    intersectionPoints.clear();
    if(continueToCapture == false){
        createCameraSetupFile();
        releaseComponents();
        ui->mainMenuWidget->show();
        continueToCapture = true;
    }
    else{
        ui->lecDet_courses->clear();
        populateCourseList();
        navigationBarUpdate(true);
        ui->lectureDetailsWidget->show();
    }
}

/// COURSE SELECTION BUTTONS
void MainWindow::on_lecDet_Continue_Button_clicked(){
    ui->lectureDetailsWidget->hide();

    if(ui->lecDet_courses->currentText() == "Other"){
        ofstream log((codePath+"/paol-code/courses.txt").c_str(), std::ios_base::app | std::ios_base::out);
        string outText = ui->new_course_textbox->text().toUtf8().constData();
        log << outText << endl; //"<< endl" to jump line
    }

    createFileDirectory();
    createCameraSetupFile();
    createInfoFile();
    for(int i = 0; i < recordingCams.length(); i++){
        recordingCams[i].release();
    }
    recordingCams.clear();
    captureVideo();
    populateCaptureWindow();
    navigationBarUpdate(true);
    ui->captureLectureWidget->show();
    videoCapture = true;
    myTimer.restart();
    myTimer.start();
}

void MainWindow::on_lecDet_Previous_Button_clicked(){
    if( whiteboards != 0){
        ui->lectureDetailsWidget->hide();
        navigationBarUpdate(false);
        ui->whiteboardCornersWidget->show();
    }
    else{
        ui->lectureDetailsWidget->hide();
        //Update twice to move backwards twice in nav bar
        navigationBarUpdate(false);
        navigationBarUpdate(false); //necessary?
        ui->setupMenuWidget->show();
    }
}

/// LECTURE CAPTURE BUTTON
void MainWindow::on_captureLecture_Terminate_Button_clicked(){
    emit quitProcessing();
    //Kill gst process through system - send SIGINT
    system("ps -ef | awk '/[g]st-launch-1.0/{print $2}' | xargs kill -INT");

    for(int i = 0; i < dev.length(); i++){
        dev[i]->wait();
    }

    ui->captureLectureWidget->hide();
    videoCapture = false;
    releaseComponents();
    // Stop FFmpeg
    //ffmpegProcess->write("q");
    //system("ps -ef | awk '/[g]st-launch-1.0/{print $2}' | xargs kill -INT"); //Moved after quitProcessing for better audio/video sync
    ffmpegProcess->closeWriteChannel();
    ffmpegProcess->waitForFinished();
    ui->mainMenuWidget->show();
}

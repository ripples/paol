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
            if(frame.channels() >= 3){ //If statement included in case a camera is connected, but not properly capturing
                if(reverseChecks[i]->isChecked())
                    flip(frame, frame, -1);
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
                thread = new VGAProcess(i, vgaCount, false, processLocation);
                vgaCount++;
            }
            else if(optionBoxes[i]->currentText() == "Whiteboard"){
                qDebug() << "Adding Whiteboard from Camera Num:" << i;
                thread = new WhiteboardProcess(i, wbCount, false, processLocation);
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


    if(continueToCapture == true){
        for(int i = 0; i < 4; i++){
            ui->locationBar->addWidget(locationText[i],0,0);
        }
    }
    else{
        for(int i = 0; i < 2; i++){
            ui->locationBar->addWidget(locationText[i],0,0);
        }
    }
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
            string flipped;

            out << i;
            num = out.str();
            out.str(string());

            out << isFlipped;
            flipped = out.str();

            setupInfo = setupInfo + num + " " + flipped + " " + device + "\n";

            if(audioRecord[i]->isChecked() == true){
                setupInfo = setupInfo + num + " 0 Audio\n";
            }

        }
    }
    //qDebug() << QString::fromUtf8(codePath+"cameraSetup.txt");
    const char *path =(codePath+"/paol-code/cameraSetup.txt").c_str();
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

    qDebug("here ");
    qDebug() << tracker;
    out << tracker;
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
    for(int h = 0; h < audioRecord.length(); h++){
        if(audioRecord[h]->isChecked()){
            out << h;
            audioCamNum = out.str();
            out.str(string());
        }
    }

    for(int i = 0; i < optionBoxes.length(); i++){
        //Find if there is a Video camera selected
        if(optionBoxes[i]->currentText() == "Video"){
            string isChecked = "0";

            if(reverseChecks[i]->isChecked() == true){
                isChecked = "1";
            }

            out << i;
            s = out.str();
            audioCamNum.erase(std::remove(audioCamNum.begin(),audioCamNum.end(),' '),audioCamNum.end());

            vidCaptureString =codePath+"/paol-code/scripts/capture/videoCapturePortable /dev/video" + s + " hw:" + audioCamNum + " "
            + isChecked + " " + processLocation + "/video.mp4 &";
            //qDebug("%s",vidCaptureString.c_str());
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
    string uploadScript=codePath+"/paol-code/scripts/upload/uploadAllPortable.sh "+codePath;
    qDebug("%s",uploadScript.c_str());
    QProcess *runUpload=new QProcess(this);
    runUpload->start(uploadScript.c_str());
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
    ffmpegProcess->write("q");
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
    /*qDebug("pair one %f %f",lineSlopes[0],lineSlopes[1]);
    qDebug("pair two %f %f",lineSlopes[2],lineSlopes[3]);
    for (int i=0;i<linePoints.length();i++){
        qDebug("xold=%d \txnew=%d",clickedCorners[i].x ,linePoints[i].x);
    }*/
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
        log << outText;
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
        navigationBarUpdate(false);
        ui->setupMenuWidget->show();
    }
}

/// LECTURE CAPTURE BUTTON
void MainWindow::on_captureLecture_Terminate_Button_clicked(){   
    emit quitProcessing();

    for(int i = 0; i < dev.length(); i++){
        dev[i]->wait();
    }

    ui->captureLectureWidget->hide();
    videoCapture = false;
    releaseComponents();
    // Stop FFmpeg
    ffmpegProcess->write("q");
    ffmpegProcess->closeWriteChannel();
    ffmpegProcess->waitForFinished();
    ui->mainMenuWidget->show();
}

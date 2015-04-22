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
        QTimer *qTimer=new QTimer(this);
        connect(qTimer,SIGNAL(timeout()),this,SLOT(launch_System()));
        connect(ui->wbc_label, SIGNAL(Mouse_Pressed()), this, SLOT(Mouse_Pressed()));
        qTimer->start(1);
    }

MainWindow::~MainWindow(){
    delete ui;
}

///////////////////////////////////////////////////////////////
///           Runner function for Capture Sys              ///
/////////////////////////////////////////////////////////////

void MainWindow::launch_System(){
    if(ui->setupMenuWidget->isVisible()){
        for(int i=0; i < camCount; i++){
            Mat frame,display;
            recordingCams[i] >> frame;
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
                //qDebug() << "Adding USB from Camera Num:" << i;
                thread = new VGAProcess(i, i, false, processLocation);
            }
            else if(optionBoxes[i]->currentText() == "Whiteboard"){
                //qDebug() << "Adding Whiteboard from Camera Num:" << i;
                thread = new WhiteboardProcess(i, i, false, processLocation);
            }

            dev.push_back(thread);

            // Associate the processing thread with the proper views in the capture window
            threadToUIMap[thread] = i;

            // Initialize the slots for updating the UI and stopping the processing threads
            connect(thread, SIGNAL(capturedImage(Mat,paolProcess*)), this, SLOT(onImageCaptured(Mat,paolProcess*)));
            connect(thread, SIGNAL(savedImage(Mat,paolProcess*)), this, SLOT(onImageSaved(Mat,paolProcess*)));
            connect(this, SIGNAL(quitProcessing()), thread, SLOT(onQuitProcessing()));
            ui->lectureCaptureGrid->addLayout(newLayout,((captureCount-1)+1) / captureDevices, ((i-1)+1) % captureDevices);
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
    std::string outFileName;
    camCount = 0;
    int temp;

    //Obtain amount of cameras connected to this device
    if ((ptr = popen("ls /dev/video*", "r")) != NULL){
        while(fgets(buf, bufSize, ptr)){
            outFileName = std::string(buf);
            outFileName = outFileName.substr(10,outFileName.find('\n'));
            outFileName = outFileName.substr(0,outFileName.find('\n'));

            std::stringstream(outFileName) >> temp;
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
        }
    }
    const char *path = "/home/paol/paol-code/cameraSetup.txt";
    ofstream file(path);
    file << setupInfo;
}

//Fills the QComboBox on the Classroom information view with available classes
void MainWindow::populateCourseList(){
    ifstream inputFile("/home/paol/paol-code/courses.txt");
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

    out << tracker;
    filePath = "/home/paol/paol-code/wbCorners" + out.str() + ".txt";
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

    system("mkdir -p /home/paol/recordings/readyToUpload");
    string firstCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + currentYear;
    system(firstCmd.data());

    string secondCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + currentYear + "/" + courseNumber;
    system(secondCmd.data());

    string thirdCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + "";
    system(thirdCmd.data());

    char buffer[80];
    time (&theTime);
    aTime = localtime(&theTime);
    strftime(buffer,80,"%m-%d-%Y--%H-%M-%S",aTime);

    string fourthCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + buffer;
    system(fourthCmd.data());

    processLocation = "/home/paol/recordings/readyToUpload/" + currentYear + "/" + courseNumber + "/" + buffer;

    string makeComputerDir = "mkdir -p " + processLocation + "/computer";
    string makeWhiteboardDir = "mkdir -p " + processLocation + "/whiteboard";
    string makeLogDir = "mkdir -p " + processLocation + "/logs";

    system(makeComputerDir.c_str());
    system(makeWhiteboardDir.c_str());
    system(makeLogDir.c_str());
}

void MainWindow::releaseComponents(){

    for(int i = 0; i < dev.length(); i ++){
        delete dev[i];
    }

    for(int i = 0; i < camLabels.length(); i++){
        delete camLabels[i];
        delete paolLabels[i];
        delete capLayouts[i];
    }
    camLabels.clear();
    paolLabels.clear();
    dev.clear();
    capLayouts.clear();

    for(int j = 0; j < recordingCams.length(); j++){
        delete imLabels[j];
        delete optionBoxes[j];
        delete reverseChecks[j];
        delete audioRecord[j];
        recordingCams[j].release();
    }
    recordingCams.clear();
    audioRecord.clear();
    reverseChecks.clear();
    optionBoxes.clear();
    imLabels.clear();
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

            vidCaptureString ="/home/paol/paol-code/scripts/capture/videoCapturePortable /dev/video" + s + " hw:" + audioCamNum + " "
            + isChecked + " " + processLocation + "/video.mp4 &";
            system(vidCaptureString.data());
        }
    }
}


//////////////////////////////////////////////////////////////
///                    Signal handling                    ///
////////////////////////////////////////////////////////////

void MainWindow::onImageCaptured(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    qDebug("Capture: Send captured image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
    int displayNum = threadToUIMap[threadAddr];
    displayMat(image, *camLabels[displayNum]);
}

void MainWindow::onImageSaved(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    qDebug("Saved: Send saved image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
    int displayNum = threadToUIMap[threadAddr];
    qDebug() << displayNum;
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
    ui->mainMenuWidget->hide();
    ui->setupMenuWidget->show();
}

void MainWindow::on_mainMenu_Setup_Cameras_clicked(){
    continueToCapture = false;
    on_mainMenu_Full_Run_clicked();
}

void MainWindow::on_mainMenu_Upload_Lectures_clicked(){

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

    if(whiteboards != 0){
        while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
            corners_currentCam += 1;
        }

        Mat frame,display;
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
    system("pkill ffmpeg");
    ui->mainMenuWidget->show();
}


/// WHITEBOARD CORNERS WINDOW BUTTONS
void MainWindow::on_WBC_PrevWB_clicked(){
    intersectionPoints.clear();
    clickedCorners.clear();
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
    intersectionPoints.clear();
    clicked = 0;
    place_image();
}

void MainWindow::on_WBC_Save_clicked(){
    if(clicked == 8){
        place_image();

        for(int i = 0; i < clickedCorners.length() - 2; i = i + 2){
            intersectionPoints.append(
                        determineIntersection(clickedCorners[i].x,clickedCorners[i].y,clickedCorners[i+1].x,clickedCorners[i+1].y,clickedCorners[i+2].x,
                                  clickedCorners[i+2].y, clickedCorners[i+3].x, clickedCorners[i+3].y));
        }

        intersectionPoints.append(
                    determineIntersection(clickedCorners[0].x,clickedCorners[0].y,clickedCorners[1].x,clickedCorners[1].y,clickedCorners[6].x,
                              clickedCorners[6].y, clickedCorners[7].x, clickedCorners[7].y));

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
    intersectionPoints.clear();
    QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::Mouse_Pressed(){
    clicked += 1;
    double posX = (1920.0 / ui->wbc_label->width()) * ui->wbc_label->x;
    double posY = (1080.0 / ui->wbc_label->height()) * ui->wbc_label->y;
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
    Mat frame,display;
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
    ui->setupMenuWidget->show();
}

void MainWindow::on_WBC_Continue_Button_clicked(){
    ui->whiteboardCornersWidget->hide();

    clickedCorners.clear();
    intersectionPoints.clear();

    if(continueToCapture == false){
        ui->mainMenuWidget->show();
        continueToCapture = true;
    }
    else{
        ui->lecDet_courses->clear();
        populateCourseList();
        ui->lectureDetailsWidget->show();
    }
}

/// COURSE SELECTION BUTTONS
void MainWindow::on_lecDet_Continue_Button_clicked(){
    ui->lectureDetailsWidget->hide();

    if(ui->lecDet_courses->currentText() == "Other"){
        ofstream log("/home/paol/paol-code/courses.txt", std::ios_base::app | std::ios_base::out);
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
    ui->captureLectureWidget->show();
    videoCapture = true;
}

void MainWindow::on_lecDet_Previous_Button_clicked(){
    if( whiteboards != 0){
        ui->lectureDetailsWidget->hide();
        ui->whiteboardCornersWidget->show();
    }
    else{
        ui->lectureDetailsWidget->hide();
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
    system("pkill ffmpeg");
    ui->mainMenuWidget->show();
}

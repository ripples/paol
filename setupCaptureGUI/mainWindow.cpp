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
    QTimer *qTimer=new QTimer(this);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(launch_System()));
    connect(ui->wbc_label, SIGNAL(Mouse_Pressed()), this, SLOT(Mouse_Pressed()));
    qTimer->start(1);
   }

MainWindow::~MainWindow(){
    delete ui;
}


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

}

void MainWindow::Mouse_Pressed(){
    if(corners_count < 4){
        //corners_count += 1;
        qDebug() << QString("X = %1, Y = %2").arg(ui->wbc_label->x).arg(ui->wbc_label->y);
        circle(corners_Clone, Point(corners_Clone.rows/2,corners_Clone.cols/2), 32.0, Scalar( 255, 255, 0 ), 50, 8);
        QImage img = QImage((const unsigned char*)(corners_Clone.data),corners_Clone.cols,corners_Clone.rows,corners_Clone.step,QImage::Format_RGB888);
        ui->wbc_label->setPixmap(QPixmap::fromImage(img));
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

        //QRadioButton *const radioButton = new QRadioButton("Should this device capture the audio",this);
        //audioRecord.push_back(radioButton);

        newLayout->addWidget(label,0,0);
        newLayout->addWidget(comboBox);
        newLayout->addWidget(checkBox);
        //newLayout->addWidget(radioButton);

        ui->setupGridLayout->addLayout(newLayout,((i-1)+1) / 3, ((i-1)+1) % 3);

        VideoCapture cap(i);

        cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
        recordingCams.push_back(cap);
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

//Create string information that will be stored to cameraSetup.txt
void MainWindow::createCameraSetupTxt(){
    cameraSetupTxt = "";
    stringstream ss;
    int compCount = 0;
    int wbCount = 0;

    //Get time since Epoch
    time_t epoch = time(0);
    ss << epoch;
    cameraSetupTxt = "timestamp: " + ss.str() + "\n";

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
    cameraSetupTxt = cameraSetupTxt + "whiteboardCount: " + ss.str() + "\n";
    ss.str(string()); // Clear out stringstream

    ss << compCount;
    cameraSetupTxt = cameraSetupTxt + "computerCount: " + ss.str() + "\n";
}

//Fills the QComboBox on the Classroom information view with available classes
void MainWindow::populateCourseList(){
    ifstream inputFile("/home/paol/Desktop/PAOL-LectureCapture-GUI-MASTER/courses.txt");
    string str;
    while(getline(inputFile, str)){
        qDebug() << str.data();
        ui->lecDet_courses->addItem(str.data());
    }
    ui->lecDet_courses->addItem("Other");
}

/*
//////////////////////////////////////////////////////////////
///                    Signal handling                    ///
////////////////////////////////////////////////////////////

void MainWindow::onImageCaptured(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    if(runCaptureCams){
        qDebug("Send captured image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
        int displayNum = threadToUIMap[threadAddr];
        displayMat(image, *camLabels[displayNum]);
    }
}

void MainWindow::onImageSaved(Mat image, paolProcess *threadAddr){
    // Only respond to the signal if the capture GUI is running
    if(runCaptureCams){
        qDebug("Send saved image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
        int displayNum = threadToUIMap[threadAddr];
        displayMat(image, *paolLabels[displayNum]);
    }
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
*/

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
    createCameraSetupTxt();
    ui->setupMenuWidget->hide();
    corners_currentCam = 0;
    corners_count = 0;

    while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
        corners_currentCam += 1;
    }

    Mat frame,display;
    recordingCams[corners_currentCam] >> frame;
    cvtColor(frame,display,CV_BGR2RGB);
    corners_Clone = display.clone();
    QImage img = QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));

    ui->whiteboardCornersWidget->show();
}

void MainWindow::on_setupReturnButton_clicked(){
    ui->setupMenuWidget->hide();

    ui->mainMenuWidget->show();
}


/// WHITEBOARD CORNERS WINDOW BUTTONS
void MainWindow::on_WBC_PrevWB_clicked(){
    corners_currentCam -= 1;

    if(corners_currentCam < 0){
        corners_currentCam = camCount - 1;
    }

    while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
        corners_currentCam -= 1;
    }

    Mat frame,display;
    recordingCams[corners_currentCam] >> frame;
    cvtColor(frame,display,CV_BGR2RGB);
    corners_Clone = display.clone();
    QImage img = QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));
    corners_count = 0;
}

void MainWindow::on_WBC_NextWB_clicked(){
    corners_currentCam += 1;

    if(corners_currentCam >= camCount){
        corners_currentCam = 0;
    }

    while(optionBoxes[corners_currentCam]->currentText() != "Whiteboard" && corners_currentCam < camCount){
        corners_currentCam += 1;
    }

    Mat frame,display;
    recordingCams[corners_currentCam] >> frame;
    cvtColor(frame,display,CV_BGR2RGB);
    corners_Clone = display.clone();
    QImage img = QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
    ui->wbc_label->setPixmap(QPixmap::fromImage(img));
    corners_count = 0;
}

void MainWindow::on_WBC_Clear_clicked(){

}

void MainWindow::on_WBC_Return_Button_clicked(){
    ui->whiteboardCornersWidget->hide();

    ui->setupMenuWidget->show();
}

void MainWindow::on_WBC_Continue_Button_clicked(){
    ui->whiteboardCornersWidget->hide();
    if(continueToCapture == false){
        ui->mainMenuWidget->show();
        continueToCapture = true;
    }
    else{
        populateCourseList();
        ui->lectureDetailsWidget->show();
    }
}


/// COURSE SELECTION BUTTONS
void MainWindow::on_lecDet_Continue_Button_clicked(){
    ui->lectureDetailsWidget->hide();
    if(ui->lecDet_courses->currentText() == "Other"){
        ofstream log("/home/paol/Desktop/PAOL-LectureCapture-GUI-MASTER/courses.txt", std::ios_base::app | std::ios_base::out);
        string outText = ui->new_course_textbox->text().toUtf8().constData();
        log << outText;
    }

    ui->captureLectureWidget->show();
}

void MainWindow::on_lecDet_Previous_Button_clicked(){
    ui->lectureDetailsWidget->hide();

    ui->whiteboardCornersWidget->show();
}


/// LECTURE CAPTURE BUTTON
void MainWindow::on_captureLecture_Terminate_Button_clicked(){
    ui->captureLectureWidget->hide();

    ui->mainMenuWidget->show();
}

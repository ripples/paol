#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    runSetupCams = false;
    runCaptureCams = false;
    ui->setupUi(this);
    QTimer *qTimer=new QTimer(this);
    findCameras();
    populateSetupWindow();
    ui->infoWidget->setVisible(false);
    ui->captureWidget->setVisible(false);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(runSystem()));
    qTimer->start(1);
}

MainWindow::~MainWindow(){
    delete ui;
    system("pkill ffmpeg");
    for(int i=0; i < count; i++){
        if(recordingCams[i].isOpened() == true){
            recordingCams[i].release();
        }
    }
}

void MainWindow::runSystem(){

    //When the user has transitioned from Info to Setup
    //Display what each connected camera currently sees
    if(runSetupCams == true){
        for(int i=0; i<count; i++){
            //Capture and display camera
            Mat frame;
            Mat display;
            recordingCams[i] >> frame;
            if(frame.channels() >= 3){
                cvtColor(frame,display,CV_BGR2RGB);
                QImage img = QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);
                imLabels[i]->setPixmap(QPixmap::fromImage(img));
            }
        }
    }

    if(runCaptureCams == true){
        // Update elapsed time
        timer();
    }
}


///////////////////////////////////////////////////////////////
///               GUI Manipulating Functions               ///
/////////////////////////////////////////////////////////////

void MainWindow::populateSetupWindow(){
    //For each camea found connected to this computer
    //Add labels to the mainwindow.ui Grid Layout
    //Currently adds three labels per row
    qDebug() << "Number of Cameras plugged in: " << count;
    for (int i = 0; i < count; i++) {
        QGridLayout *const newLayout = new QGridLayout;
        setupLayouts.push_back(newLayout);
        //Create label that will contain image
        QLabel *const label = new QLabel(QString("CAM %1").arg(i, 2, 10, QLatin1Char('0')));
        label->setScaledContents(true);
        imLabels.push_back(label); //Add created label to vector

        //Create ComboBox containing all video capture options
        QComboBox *const comboBox = new QComboBox;
        comboBox->addItem("Blank");
        comboBox->addItem("Whiteboard");
        comboBox->addItem("VGA2USB");
        comboBox->addItem("Video");
        optionBoxes.push_back(comboBox); //Add created box to vector

        //Create Check Box for whether to reverse camera or not
        QCheckBox *const checkBox = new QCheckBox("Check if image is reversed",this);
        reverseChecks.push_back(checkBox);

        //Create Radio Button to confirm if camera will capture audio
        QRadioButton *const radioButton = new QRadioButton("Select if this device will capture audio",this);
        audioRecord.push_back(radioButton);

        //Add all created widgets to the layout
        newLayout->addWidget(label,0,0);
        newLayout->addWidget(comboBox);
        newLayout->addWidget(checkBox);
        newLayout->addWidget(radioButton);

        //Attach layout to setup grid
        ui->setupCameraGrid->addLayout(newLayout,((i-1)+1) / 3, ((i-1)+1) % 3);

        VideoCapture cap(i);

        cap.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
        recordingCams.push_back(cap);
    }
    //Default to setting first camera to record audio
    audioRecord[0]->setChecked(true);

    runSetupCams = true;
}

void MainWindow::populateCaptureWindow(){
    captureCount = 0;
    qDebug() << "Value of Count in CaptureWindow: " << count;
    for(int i = 0; i < count; i++){
        qDebug() << "On Number: " << i;
        string compare = camTypes[i].toUtf8().constData();
        if(compare != "Video" && compare != "Blank"){
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

            // Required to be able to signal with Mats
            qRegisterMetaType<Mat>("Mat");

            paolProcess* thread;
            if(compare == "VGA2USB"){
                qDebug() << "Adding USB from Camera Num:" << i;
                thread = new VGAProcess(i, i, false, processLocation);
            }else if(compare == "Whiteboard"){
                qDebug() << "Adding Whiteboard from Camera Num:" << i;
                thread = new WhiteboardProcess(i, i, false, processLocation);
            }
            dev.push_back(thread);
            // Associate the processing thread with the proper views in the capture window
            threadToUIMap[thread] = captureCount;

            // Initialize the slots for updating the UI and stopping the processing threads
            connect(thread, SIGNAL(capturedImage(Mat,paolProcess*)), this, SLOT(onImageCaptured(Mat,paolProcess*)));
            connect(thread, SIGNAL(processedImage(Mat,paolProcess*)), this, SLOT(onImageProcessed(Mat,paolProcess*)));
            connect(this, SIGNAL(quitProcessing()), thread, SLOT(onQuitProcessing()));

            ui->captureCameraGrid->addLayout(newLayout,((captureCount-1)+1) / 3, ((i-1)+1) % 3); //COPY AND MOVE UP
            captureCount++;
        }
    }
}


///////////////////////////////////////////////////////////////
///                     Utilities                          ///
/////////////////////////////////////////////////////////////

//Updates count based on how many cameras are attached to the machine
void MainWindow::findCameras(){
    int bufSize = 512;
    char *buf = new char[bufSize];
    FILE *ptr;
    std::string outFileName;
    count = 0;
    int temp;

    //Obtain amount of cameras connected to this device
    if ((ptr = popen("ls /dev/video*", "r")) != NULL){
        while(fgets(buf, bufSize, ptr)){
            outFileName = std::string(buf);
            outFileName = outFileName.substr(10,outFileName.find('\n'));
            outFileName = outFileName.substr(0,outFileName.find('\n'));

            std::stringstream(outFileName) >> temp;
            count++;
        }
        fclose(ptr);
    }
    fullCount = count;
}

void MainWindow::createCamDocument(){
    qDebug() << "Value of Count in CamDocument: " << count;
    string outputInfo = "";
    for(int l = 0; l < optionBoxes.size(); l++){
        QString text = optionBoxes[l]->currentText(); //Acquires user selected option from ComboBox
        string device = text.toLatin1().data();       //Convert QString to string

        int isFlipped = 0;
        if(device != "Blank"){
            if(reverseChecks[l]->isChecked() == true){
                isFlipped = 1;
            }
            //String to contain converted integer
            string s; //Camera Number
            string i; //Reversed or not

            //string stream used to convert int to string
            stringstream out;
            stringstream out2;

            //Nothing is ever convinient.
            out << l;
            s = out.str();
            out2 << isFlipped;
            i = out2.str();

            outputInfo = outputInfo + s + " " + i + " " + device + "\n";

            //Adds which device is responsible for recording audio outputInfo
            if(audioRecord[l]->isChecked()){
                string a; //Audio Cam Num string
                stringstream out3; //Stringstream to convert int to str
                out3 << l;
                a = out3.str();
                audioCamNum = a;
                outputInfo = outputInfo + a + " 0 " + "Audio" + "\n";
            }
        }
    }
    //outputInfo = outputInfo + ui->audioCamBox->currentText().toLatin1().data() + " Audio" + "\n";
    const char *path = "/home/paol/paol-code/cameraSetup.txt";
    //Creates .txt file to which outputInfo is placed in
    //std::cout << outputInfo;
    std::ofstream file(path);
    file << outputInfo;
}

bool MainWindow::courseInformation(){
    QString classText = ui->captureCourse->text();
    QString yearText = ui->captureSemester->text();
    string classIn = classText.toLatin1().data();
    string yearIn = yearText.toLatin1().data();
    //Makes sure the user has entered text in the input fields before proceeding to create folders
    if(classIn.length() >= 4 && yearIn.length() >= 4){
        //Create directory for saving processed images
        system("mkdir -p /home/paol/recordings/readyToUpload");
        string firstCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + yearIn;
        system(firstCmd.data());

        string secondCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + yearIn + "/" + classIn;
        system(secondCmd.data());

        string thirdCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + yearIn + "/" + classIn + "/" + "";
        system(thirdCmd.data());

        //Get time from system
        time_t rawtime;
        struct tm * timeinfo;
        char buffer[80];

        time (&rawtime);
        timeinfo = localtime(&rawtime);
        strftime(buffer,80,"%m-%d-%Y--%H-%M-%S",timeinfo);

        string fourthCmd = "mkdir -p /home/paol/recordings/readyToUpload/" + yearIn + "/" + classIn + "/" + buffer;
        system(fourthCmd.data());

        processLocation = "/home/paol/recordings/readyToUpload/" + yearIn + "/" + classIn + "/" + buffer;

        ui->infoWidget->setVisible(false);
        ui->captureWidget->setVisible(true);
        return true;
    }
    else{
        //If the user hasn't entered any information into the subject boxes, show them an error
        ui->error_label->setStyleSheet("QLabel {color:red}");

        //If the Course Name length is between 0 and 3
        QString error = "";
        if(classIn.length() == 0){
            error = error + "You have not entered anything into the 'Subject' box \n";
        }
        else{
            error = error + "The class length is too short \n";
        }
        //If the Year Length is between 0 and 4
        if(yearIn.length() == 0){
            error = error + "You have not entered anything into the 'Semester' box \n";
        }
        else{
            error = error + "The semester length is too short \n";
        }
        ui->error_label->setText(error);
        return false;
    }
}

void MainWindow::configureCaptureSettings(){
    bool videoExists = false;
    qDebug() << "Value of Count in ConfigureCapture: " << count;
    for(int i = 0; i < count; i++){
        //Find if there is a Video camera selected
        if(optionBoxes[i]->currentText() == "Video" && videoExists == false){
            videoExists = true;
            string isChecked = "0";
            if(reverseChecks[i]->isChecked() == true){
                isChecked = "1";
            }
            string s;
            stringstream out;
            out << i;
            s = out.str();

            audioCamNum.erase(std::remove(audioCamNum.begin(),audioCamNum.end(),' '),audioCamNum.end());

            vidCaptureString ="/home/paol/paol-code/scripts/capture/videoCapturePortable /dev/video" + s + " hw:" + audioCamNum + " "
                                + isChecked + " " + processLocation + "/video1.mp4 &";

            //recordingCams[i].release();
            //recordingCams.remove(i);
        }
    }
}

void MainWindow::releaseSetupElements(){
    runSetupCams = false;
    for(int i = 0; i < count; i ++){
        recordingCams[i].release();
        delete imLabels[i];
        camTypes.append(optionBoxes[i]->currentText());
        delete optionBoxes[i];
        delete reverseChecks[i];
        delete audioRecord[i];
    }
    recordingCams.clear();
    imLabels.clear();
    optionBoxes.clear();
    reverseChecks.clear();
    audioRecord.clear();
}

void MainWindow::releaseCaptureElements(){
    system("pkill ffmpeg");
    vidCaptureString = "";
    qDebug() << vidCaptureString.data();
    for(int i = 0; i < captureCount; i++){
        delete camLabels[i];
        delete paolLabels[i];
        delete capLayouts[i];
        delete dev[i];
    }
    camLabels.clear();
    paolLabels.clear();
    capLayouts.clear();
    dev.clear();
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
    ui->captureTimer->setText(QString::fromStdString(final));
}

//////////////////////////////////////////////////////////////
///  Button Functions used to transition between windows  ///
////////////////////////////////////////////////////////////

//Refresh GUI elements based on how many cameras may have been added or removed
void MainWindow::on_setupRefreshCameras_clicked(){
    releaseSetupElements();
    findCameras();
    populateSetupWindow();
}

//Transition from Setup to Info
void MainWindow::on_setupContinueToCapture_clicked(){
    ui->setupWidget->setVisible(false);
    createCamDocument();
    ui->infoWidget->setVisible(true);
}

//Begins processing and runs FFMPEG
void MainWindow::on_infoContinue_clicked(){
    isVideo = courseInformation();
    configureCaptureSettings();
    releaseSetupElements();
    if(isVideo == true){
        //Run FFMPEG Command, string created in courseInformation
        if(vidCaptureString.length() > 1){
            system(vidCaptureString.data());
        }
        populateCaptureWindow();
        runCaptureCams = true;
        // Start timer and capture threads
        for(int i = 0; i < captureCount; i++)
            dev[i]->start();
        myTimer.start();
    }
}

//Transition from Information to Setup
void MainWindow::on_infoReturnToSetup_clicked(){
    releaseSetupElements();
    ui->infoWidget->setVisible(false);
    populateSetupWindow();
    ui->setupWidget->setVisible(true);
}

//Transition from Capture to Setup
void MainWindow::on_captureReturnToSetup_clicked(){
    runCaptureCams = false;

    // Kill ffmpeg
    system("q");

    // Signal the threads to stop and wait for them
    emit quitProcessing();
    for(int i = 0; i < captureCount; i++) {
        dev[i]->wait();
    }

    ui->captureWidget->setVisible(false);
    releaseCaptureElements();
    findCameras();
    populateSetupWindow();
    qDebug() << "Total time elapsed (in seconds): " << captureSecondsElapsed;
    myTimer.restart();
    ui->setupWidget->setVisible(true);
}


void MainWindow::on_setupUploadFiles_clicked(){
    system("/home/paol/paol-code/scripts/upload/upload.sh");
}


//////////////////////////////////////////////////////////////
///                    Signal handling                    ///
////////////////////////////////////////////////////////////

void MainWindow::onImageCaptured(Mat image, paolProcess *threadAddr) {
    // Only respond to the signal if the capture GUI is running
    if(runCaptureCams) {
        qDebug("Send captured image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
        int displayNum = threadToUIMap[threadAddr];
        displayMat(image, *camLabels[displayNum]);
    }
}

void MainWindow::onImageProcessed(Mat image, paolProcess *threadAddr) {
    // Only respond to the signal if the capture GUI is running
    if(runCaptureCams) {
        qDebug("Send processed image from thread %p to display %d", threadAddr, threadToUIMap[threadAddr]);
        int displayNum = threadToUIMap[threadAddr];
        displayMat(image, *paolLabels[displayNum]);
    }
}

//////////////////////////////////////////////////////////////
///                    Image displaying                   ///
////////////////////////////////////////////////////////////

QImage MainWindow::convertMatToQImage(const Mat& mat) {
    Mat display;
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mat,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888)
            .copy();
    return img;
}

void MainWindow::displayMat(const Mat& mat, QLabel &location) {
    //call method to convert Mat to QImage
    QImage img=convertMatToQImage(mat);
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

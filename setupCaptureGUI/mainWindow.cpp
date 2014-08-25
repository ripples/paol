#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow){
    ui->setupUi(this);
    QTimer *qTimer=new QTimer(this);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(beginRun()));
    qTimer->start(2000);
    createdUI = false;
}

MainWindow::~MainWindow(){
    delete ui;
}


void MainWindow::beginRun(){
    if(createdUI == false){
        populateGUI();
        createdUI = !createdUI;
    }
    for (int i = 0; i < imLabels.size(); i++){
        dev[i]->takePicture();
        if(reverseChecks[i]->isChecked() == true){
            dev[i]->flipPicture();
        }
        dev[i]->displayImage(*imLabels[i]);
    }
}

//Function Name: populateGUI
//Purpose: Places QLabel and QComboBox objects on the ui based
//         on how many cameras are attached to device
//Output: None
//Properties Modified: UI MainGridLayout
void MainWindow::populateGUI(){
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

            dev.push_back(new paolMat());
            //background.push_back(new paolMat());
            //backgroundRefined.push_back(new paolMat());
            //oldBackgroundRefined.push_back(new paolMat());
            //old.push_back(new paolMat());
            //rawEnhanced.push_back(new paolMat());

            dev[count]->setCameraNum(temp);
            dev[count]->takePicture();
            //background[count]->copy(dev[count]);
            //backgroundRefined[count]->copyClean(dev[count]);
            //oldBackgroundRefined[count]->copyClean(dev[count]);

            count++;

            qDebug("%d",temp);
        }
        fclose(ptr);
    }

    //For each camea found connected to this device
    //Add labels to the mainwindow.ui Grid Layout
    //Currently adds three labels per row
    for (int i = 0; i < count; ++ i) {
        QGridLayout *const newLayout = new QGridLayout;

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

        newLayout->addWidget(label,0,0);
        newLayout->addWidget(comboBox);
        newLayout->addWidget(checkBox);
        newLayout->addWidget(radioButton);

        ui->cameraGrid->addLayout(newLayout,((i-1)+1) / 3, ((i-1)+1) % 3);

    }
}

//Function Name: on_confirmBttn_clicked
//Purpose: Creates and outputs a .txt file based on which
//         options in the UI ComboBoxes were selected
//Output: *.txt file
//Properties Modified: None
void MainWindow::on_confirmBttn_clicked(){
    outputInfo = "";
    for(int l = 0; l < optionBoxes.size(); l++){
        QString text = optionBoxes[l]->currentText(); //Acquires user selected option from ComboBox
        string device = text.toLatin1().data(); //Convert QString to string

        int isFlipped = 0;
        if(device != "Blank"){
            if(reverseChecks[l]->isChecked() == true){
                isFlipped = 1;
            }
            //String to contain converted integer
            std::string s; //Camera Number
            std::string i; //Reversed or not

            //string stream used to convert int to string
            std::stringstream out;
            std::stringstream out2;

            //Nothing is ever convinient.
            out << l;
            s = out.str();
            out2 << isFlipped;
            i = out2.str();

            outputInfo = outputInfo + s + " " + i + " " + device + "\n";

            //Adds which device is responsible for recording audio outputInfo
            if(audioRecord[l]->isChecked()){
                std::string a; //Audio Cam Num string
                std::stringstream out3; //Stringstream to convert int to str
                out3 << l;
                a = out3.str();
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


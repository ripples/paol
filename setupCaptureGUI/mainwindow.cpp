#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    QTimer *qTimer=new QTimer(this);
    connect(qTimer,SIGNAL(timeout()),this,SLOT(displayCam()));
    qTimer->start(2000);

    createdUI = false;
    captureOn = false;
    ui->cameraWidget->setVisible(false);
    ui->captureWidget->setVisible(false);
}

//MainWindow Deconstructor
MainWindow::~MainWindow(){
    delete ui;
}

//Function Name: displayCame
//Purpose: Displays images captured by connected cameras in
//         their respective labels
//Output: None
//Properties Modified: Individual imLabels
void MainWindow::displayCam(){

    if(createdUI == false){
        populateGUI();
        createdUI = !createdUI;
    }

    if(ui->cameraWidget->isVisible() == true){
        for (int i = 0; i < imLabels.size(); i++){
            dev[i]->takePicture();
            dev[i]->displayImage(*imLabels[i]);
        }
    }

    if(ui->captureWidget->isVisible() == true){
        if(captureOn == true){
            for (uint i = 0; i < camNums.size(); i++){
                old[camNums[i]]->copy(dev[camNums[i]]);
                dev[camNums[i]]->takePicture();
                //dev[camNums[i]]->displayImage(*imLabels2[i]);
            }
            processWhiteboards();
            processComputer();
        }
    }
}

void MainWindow::processWhiteboards(){
    for (uint i = 0; i < camNums.size(); i++){
        if(camNames[camNums[i]]=="Whiteboard"){

            //compare picture to previous picture and store differences in old->maskMin
            numDif=old[camNums[i]]->differenceMin(dev[camNums[i]],40,1);
    //qDebug(" numDif=%f\n",numDif);

            //if there is enough of a difference between the two images
            if(numDif>.03){
                //set up a new % that represents difference
                refinedNumDif=old[camNums[i]]->shrinkMaskMin();
                count=0;
            } else {
                refinedNumDif=0;
            }

            //if the images are really identical, count the number of consecultive nearly identical images
            if (numDif < .000001)
                count++;

 //qDebug(" refinedNumDif=%f  numDif=%f\n",refinedNumDif,numDif);

            //if the differences are enough that we know where the lecturer is or the images have been identical
            //for two frames, and hence no lecturer present
            if(refinedNumDif>.04 || (numDif <.000001 && count==2)){
                //copy the input image and process it to highlight the text
                rawEnhanced[camNums[i]]->copy(dev[camNums[i]]);
                rawEnhanced[camNums[i]]->averageWhiteboard(20);
                rawEnhanced[camNums[i]]->enhanceText();
                //rawEnhanced[camNums[i]]->displayImage(*ui[camNums[i]]->imDisplay8);

                /////////////////////////////////////////////////////////////
                //identify where motion is

                //extend the area of differences and sweep differences for more solid area
                old[camNums[i]]->extendMaskMinToEdges();
                old[camNums[i]]->sweepDownMin();
                //keep only the solid area and grow that region
                old[camNums[i]]->keepWhiteMaskMin();
                old[camNums[i]]->growMin(8);
                //draw a convex hull around area of differences
                old[camNums[i]]->findContoursMaskMin();
                //fill in area surrounded by convex hull
                old[camNums[i]]->sweepDownMin();
                old[camNums[i]]->keepWhiteMaskMin();
                ///////////////////////////////////////////////////////////////////////

                //process to identify text location

                //smooth image
                dev[camNums[i]]->blur(1);
                //find edge information and store total edge information in 0 (blue) color channel of mask
                dev[camNums[i]]->pDrift();
                //grow the area around where the edges are found (if edge in channel 0 grow in channel 2)
                dev[camNums[i]]->grow(15,3);
                ////////////////////////////////////////////////////////////////////////////////

                //process to update background image

                //copy movement information into rawEnhanced and then expand to full mask
                rawEnhanced[camNums[i]]->copyMaskMin(old[camNums[i]]);
                rawEnhanced[camNums[i]]->maskMinToMaskBinary();

                //update the background image with new information
                background[camNums[i]]->updateBack2(rawEnhanced[camNums[i]],dev[camNums[i]]);

                //copy the background image to one for processing
                backgroundRefined[camNums[i]]->copy(background[camNums[i]]);
                //darken text and set whiteboard to white
      //backgroundRefined[camNums[i]]->displayMask(*imLabels2[i]);
                backgroundRefined[camNums[i]]->darkenText();
                //copy text location information into mask
                backgroundRefined[camNums[i]]->copyMask(background[camNums[i]]);
                //////////////////////////////////////////////////

                //figure out if saves need to be made

                //count the number of differences in the refined text area between refined images
                saveNumDif = oldBackgroundRefined[camNums[i]]->countDifsMask(backgroundRefined[camNums[i]]);
                if (saveNumDif>.004)
                    backgroundRefined[camNums[i]]->displayImage(*imLabels2[i]);
        //qDebug("save dif=%f",saveNumDif);
                //NOTE: once the image has been cut down to just white board saveNumDif should be a way
                //of determining what to save, write now crap from wall background causes it to be useless

                //oldBackgroundRefined->displayMask(*ui->imDisplay12);
                //copy last clean whiteboard image
                oldBackgroundRefined[camNums[i]]->copy(backgroundRefined[camNums[i]]);
            }
        }
    }
}

void MainWindow::processComputer(){
    int repeat=3;
    int bottomMask = 115;
    float thresholdDiff = .0002;

    for (uint i = 0; i < camNums.size(); i++){
        if(camNames[camNums[i]]=="VGA2USB"){
            if(old[camNums[i]]->src.rows==dev[camNums[i]]->src.rows && old[camNums[i]]->src.cols==dev[camNums[i]]->src.cols){
                dev[camNums[i]]->difference(old[camNums[i]], 100, 0, bottomMask);
                //percentDifference is (all the differences)/(size of the image)
                percentDifference=(double)dev[camNums[i]]->difs/(double)(dev[camNums[i]]->src.rows*dev[camNums[i]]->src.cols);
            } else {
                percentDifference=1;
            }

            //if percentDifference is greater than the threshold
   //qDebug("percentDif=%f countStable=%d",percentDifference,countStable);
            if(percentDifference>=thresholdDiff){
                //then if the number of identical images is greater then or equal to 3
                if (countStable>=repeat){
                    //save image
                    old[camNums[i]]->displayImage(*imLabels2[i]);
                }

                countStable=0;
                timeDif=dev[camNums[i]]->time;
            } else {
                countStable++;
            }
        }
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
            outFileName=std::string(buf);
            outFileName=outFileName.substr(10,outFileName.find('\n'));
            outFileName=outFileName.substr(0,outFileName.find('\n'));

            std::stringstream(outFileName) >> temp;

            dev.push_back(new paolMat());
            background.push_back(new paolMat());
            backgroundRefined.push_back(new paolMat());
            oldBackgroundRefined.push_back(new paolMat());
            old.push_back(new paolMat());
            rawEnhanced.push_back(new paolMat());

            dev[count]->setCameraNum(temp);
            dev[count]->takePicture();
            background[count]->copy(dev[count]);
            backgroundRefined[count]->copyClean(dev[count]);
            oldBackgroundRefined[count]->copyClean(dev[count]);

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

        //Create ComboBox containing all video capture options
        QComboBox *const comboBox = new QComboBox;
        comboBox->addItem("Blank");
        comboBox->addItem("Whiteboard");
        comboBox->addItem("VGA2USB");
        comboBox->addItem("Video");
        optionBoxes.push_back(comboBox); //Add created box to vector

        //Create label that will contain image
        QLabel *const label = new QLabel(QString("CAM %1").arg(i, 2, 10, QLatin1Char('0')));
        label->setScaledContents(true);
        imLabels.push_back(label); //Add created label to vector

        newLayout->addWidget(label);
        newLayout->addWidget(comboBox);

        ui->mainGridLayout->addLayout(newLayout,((i-1)+1) / 3, ((i-1)+1) % 3);

        std::string s; //String to contain converted integer
        std::stringstream out; //string stream used to convert int to string
        out << i;
        s = out.str();
        //ui->audioCamBox->addItem(s.c_str());
    } 
}

//Function Name: on_generateButton_clicked
//Purpose: Creates and outputs a .txt file based on which
//         options in the UI ComboBoxes were selected
//Output: *.txt file
//Properties Modified: None

void MainWindow::on_generateButton_clicked(){
    outputInfo = "";
    for(int l = 0; l < optionBoxes.size(); l++){
        QString text = optionBoxes[l]->currentText(); //Acquires user selected option from ComboBox
        string device = text.toLatin1().data(); //Convert QString to string

        if(device != "Blank"){
            std::string s; //String to contain converted integer
            std::stringstream out; //string stream used to convert int to string
            out << l;
            s = out.str();
            outputInfo = outputInfo + s + " " + device + "\n";
        }
    }
    //outputInfo = outputInfo + ui->audioCamBox->currentText().toLatin1().data() + " Audio" + "\n";
    const char *path = "/home/paol/paol-code/cameraSetup.txt";
    //Creates .txt file to which outputInfo is placed in
    //std::cout << outputInfo;
    std::ofstream file(path);
    file << outputInfo;


    ui->cameraWidget->setVisible(false);
    ui->buttonWidget->setVisible(true);
}

//Function Name: on_startCaptureBttn_clicked
//Purpose: Creates a proper UI layout based on which
//         cameras were selected as VGA2USB and Whiteboard
//Output:  None
//Properties Modified: camNames, camType, camNums, captureCamGrid,

void MainWindow::on_startCaptureBttn_clicked(){

    QFile cameraFile("/home/paol/paol-code/cameraSetup.txt");
    if(cameraFile.open(QIODevice::ReadOnly)){
        QTextStream in(&cameraFile);
        while(!in.atEnd()){ //Opens the cameraSetup.txt to figure out which cameras will be displayed
            QString line = in.readLine();
            string stringLine = line.toLatin1().data();
            string output = "";
            std::stringstream test(stringLine);
            while(std::getline(test,output, ' ')){
                camType.push_back(output.data());
            }
        }
    }

    for(uint j = 0; j < camType.size(); j++){
        if(j%2 != 0){
            camNames.push_back(camType[j].data());
        }
    }

    int tempNum = 0;

    for(uint m = 0; m < camNames.size(); m++){
        if(camNames[m] == "Video"){
            tempNum++;
        }
        else if (camNames[m] == "Audio"){
            tempNum = tempNum;
        }
        else{
            camNums.push_back(tempNum);
            tempNum++;
        }
    }

    for(uint c = 0; c < camNums.size(); c++){
        QGridLayout *const newLayout = new QGridLayout;
        QLabel *const label = new QLabel(QString("CAM %1").arg(c, 2, 10, QLatin1Char('0')));

        label->setScaledContents(true);
        imLabels2.push_back(label);
        newLayout->addWidget(label);
        ui->captureCamGrid->addLayout(newLayout,((c-1)+1) / 2, ((c-1)+1) % 2);
    }
    captureOn = true;
    countStable = 0;

    for (uint i = 0; i < camNums.size(); i++){
        dev[camNums[i]]->takePicture();
        //dev[camNums[i]]->displayImage(*imLabels2[i]);
    }
}

//METHODS TO SWITCH ON AND OFF CURRENT VIEWS BASED ON WHICH BUTTON HAS BEEN CLICKED
void MainWindow::on_cameraSetupBttn_clicked(){
    ui->buttonWidget->setVisible(false);
    ui->cameraWidget->setVisible(true);
}

void MainWindow::on_beginCaptureBttn_clicked(){
    ui->buttonWidget->setVisible(false);
    ui->captureWidget->setVisible(true);
}



void MainWindow::on_stopCaptureBttn_clicked()
{
    captureOn=false;
    for (uint i = 0; i < camNums.size(); i++){
        if(camNames[i]=="VGA2USB"){
            old[camNums[i]]->displayImage(*imLabels2[i]);
        }
    }
}

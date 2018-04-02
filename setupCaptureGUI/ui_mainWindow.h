/********************************************************************************
** Form generated from reading UI file 'mainWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.3.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <clickable_label.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QWidget *mainMenuWidget;
    QPushButton *mainMenu_Full_Run;
    QPushButton *mainMenu_Upload_Lectures;
    QPushButton *mainMenu_Setup_Cameras;
    QWidget *setupMenuWidget;
    QWidget *gridLayoutWidget;
    QGridLayout *setupGridLayout;
    QWidget *horizontalLayoutWidget;
    QHBoxLayout *Setup_Navigation;
    QPushButton *setupReturnButton;
    QLabel *device_setup_label;
    QLabel *wb_corners_label;
    QLabel *lec_details_label;
    QLabel *cap_lec_label;
    QPushButton *setupContinueButton;
    QLabel *setup_titleLabel;
    QWidget *whiteboardCornersWidget;
    QWidget *horizontalLayoutWidget_2;
    QHBoxLayout *WB_Navigation;
    QPushButton *WBC_Return_Button;
    QLabel *wb_corners_label_1;
    QLabel *wb_corners_label_2;
    QLabel *wb_corners_label_3;
    QLabel *wb_corners_label_4;
    QPushButton *WBC_Continue_Button;
    QWidget *horizontalLayoutWidget_3;
    QHBoxLayout *WB_Controls;
    QPushButton *WBC_PrevWB;
    QPushButton *WBC_Clear;
    QPushButton *WBC_Save;
    QPushButton *WBC_NextWB;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    clickable_label *wbc_label;
    QLabel *wbc_title;
    QWidget *lectureDetailsWidget;
    QLabel *lectureDetails_label;
    QWidget *horizontalLayoutWidget_4;
    QHBoxLayout *lectureDetails_Navigation;
    QPushButton *lecDet_Previous_Button;
    QLabel *lec_details_label_1;
    QLabel *lec_details_label_2;
    QLabel *lec_details_label_3;
    QLabel *lec_details_label_4;
    QPushButton *lecDet_Continue_Button;
    QLabel *label;
    QLabel *label_2;
    QLabel *newCourseLabel;
    QComboBox *lecDet_courses;
    QLineEdit *new_course_textbox;
    QWidget *captureLectureWidget;
    QLabel *label_4;
    QWidget *horizontalLayoutWidget_5;
    QHBoxLayout *captureLecture_Navigation;
    QLabel *capture_details_1;
    QLabel *capture_details_2;
    QLabel *capture_details_3;
    QLabel *capture_details_4;
    QWidget *horizontalLayoutWidget_6;
    QHBoxLayout *captureLecture_Info;
    QLabel *captureLecture_Timer;
    QLabel *captureLecture_Video_Status;
    QPushButton *captureLecture_Terminate_Button;
    QWidget *gridLayoutWidget_2;
    QGridLayout *lectureCaptureGrid;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1037, 555);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        mainMenuWidget = new QWidget(centralWidget);
        mainMenuWidget->setObjectName(QStringLiteral("mainMenuWidget"));
        mainMenuWidget->setGeometry(QRect(10, 5, 1020, 480));
        mainMenu_Full_Run = new QPushButton(mainMenuWidget);
        mainMenu_Full_Run->setObjectName(QStringLiteral("mainMenu_Full_Run"));
        mainMenu_Full_Run->setGeometry(QRect(10, 10, 491, 470));
        mainMenu_Upload_Lectures = new QPushButton(mainMenuWidget);
        mainMenu_Upload_Lectures->setObjectName(QStringLiteral("mainMenu_Upload_Lectures"));
        mainMenu_Upload_Lectures->setGeometry(QRect(510, 250, 510, 230));
        mainMenu_Setup_Cameras = new QPushButton(mainMenuWidget);
        mainMenu_Setup_Cameras->setObjectName(QStringLiteral("mainMenu_Setup_Cameras"));
        mainMenu_Setup_Cameras->setGeometry(QRect(510, 10, 510, 230));
        setupMenuWidget = new QWidget(centralWidget);
        setupMenuWidget->setObjectName(QStringLiteral("setupMenuWidget"));
        setupMenuWidget->setEnabled(true);
        setupMenuWidget->setGeometry(QRect(10, 5, 1020, 480));
        gridLayoutWidget = new QWidget(setupMenuWidget);
        gridLayoutWidget->setObjectName(QStringLiteral("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(10, 40, 1001, 401));
        setupGridLayout = new QGridLayout(gridLayoutWidget);
        setupGridLayout->setSpacing(6);
        setupGridLayout->setContentsMargins(11, 11, 11, 11);
        setupGridLayout->setObjectName(QStringLiteral("setupGridLayout"));
        setupGridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayoutWidget = new QWidget(setupMenuWidget);
        horizontalLayoutWidget->setObjectName(QStringLiteral("horizontalLayoutWidget"));
        horizontalLayoutWidget->setGeometry(QRect(10, 450, 1001, 29));
        Setup_Navigation = new QHBoxLayout(horizontalLayoutWidget);
        Setup_Navigation->setSpacing(6);
        Setup_Navigation->setContentsMargins(11, 11, 11, 11);
        Setup_Navigation->setObjectName(QStringLiteral("Setup_Navigation"));
        Setup_Navigation->setContentsMargins(0, 0, 0, 0);
        setupReturnButton = new QPushButton(horizontalLayoutWidget);
        setupReturnButton->setObjectName(QStringLiteral("setupReturnButton"));

        Setup_Navigation->addWidget(setupReturnButton);

        device_setup_label = new QLabel(horizontalLayoutWidget);
        device_setup_label->setObjectName(QStringLiteral("device_setup_label"));
        QFont font;
        font.setBold(true);
        font.setWeight(75);
        device_setup_label->setFont(font);
        device_setup_label->setAlignment(Qt::AlignCenter);

        Setup_Navigation->addWidget(device_setup_label);

        wb_corners_label = new QLabel(horizontalLayoutWidget);
        wb_corners_label->setObjectName(QStringLiteral("wb_corners_label"));
        wb_corners_label->setAlignment(Qt::AlignCenter);

        Setup_Navigation->addWidget(wb_corners_label);

        lec_details_label = new QLabel(horizontalLayoutWidget);
        lec_details_label->setObjectName(QStringLiteral("lec_details_label"));
        lec_details_label->setAlignment(Qt::AlignCenter);

        Setup_Navigation->addWidget(lec_details_label);

        cap_lec_label = new QLabel(horizontalLayoutWidget);
        cap_lec_label->setObjectName(QStringLiteral("cap_lec_label"));
        QFont font1;
        font1.setUnderline(false);
        cap_lec_label->setFont(font1);
        cap_lec_label->setFrameShape(QFrame::NoFrame);
        cap_lec_label->setFrameShadow(QFrame::Raised);
        cap_lec_label->setAlignment(Qt::AlignCenter);

        Setup_Navigation->addWidget(cap_lec_label);

        setupContinueButton = new QPushButton(horizontalLayoutWidget);
        setupContinueButton->setObjectName(QStringLiteral("setupContinueButton"));

        Setup_Navigation->addWidget(setupContinueButton);

        setup_titleLabel = new QLabel(setupMenuWidget);
        setup_titleLabel->setObjectName(QStringLiteral("setup_titleLabel"));
        setup_titleLabel->setGeometry(QRect(380, 10, 210, 25));
        QFont font2;
        font2.setPointSize(16);
        font2.setBold(true);
        font2.setWeight(75);
        setup_titleLabel->setFont(font2);
        setup_titleLabel->setAlignment(Qt::AlignCenter);
        whiteboardCornersWidget = new QWidget(centralWidget);
        whiteboardCornersWidget->setObjectName(QStringLiteral("whiteboardCornersWidget"));
        whiteboardCornersWidget->setGeometry(QRect(10, 5, 1020, 480));
        horizontalLayoutWidget_2 = new QWidget(whiteboardCornersWidget);
        horizontalLayoutWidget_2->setObjectName(QStringLiteral("horizontalLayoutWidget_2"));
        horizontalLayoutWidget_2->setGeometry(QRect(10, 450, 1001, 29));
        WB_Navigation = new QHBoxLayout(horizontalLayoutWidget_2);
        WB_Navigation->setSpacing(6);
        WB_Navigation->setContentsMargins(11, 11, 11, 11);
        WB_Navigation->setObjectName(QStringLiteral("WB_Navigation"));
        WB_Navigation->setContentsMargins(0, 0, 0, 0);
        WBC_Return_Button = new QPushButton(horizontalLayoutWidget_2);
        WBC_Return_Button->setObjectName(QStringLiteral("WBC_Return_Button"));

        WB_Navigation->addWidget(WBC_Return_Button);

        wb_corners_label_1 = new QLabel(horizontalLayoutWidget_2);
        wb_corners_label_1->setObjectName(QStringLiteral("wb_corners_label_1"));
        QFont font3;
        font3.setBold(false);
        font3.setWeight(50);
        wb_corners_label_1->setFont(font3);
        wb_corners_label_1->setAlignment(Qt::AlignCenter);

        WB_Navigation->addWidget(wb_corners_label_1);

        wb_corners_label_2 = new QLabel(horizontalLayoutWidget_2);
        wb_corners_label_2->setObjectName(QStringLiteral("wb_corners_label_2"));
        QFont font4;
        font4.setBold(true);
        font4.setItalic(false);
        font4.setWeight(75);
        wb_corners_label_2->setFont(font4);
        wb_corners_label_2->setAlignment(Qt::AlignCenter);

        WB_Navigation->addWidget(wb_corners_label_2);

        wb_corners_label_3 = new QLabel(horizontalLayoutWidget_2);
        wb_corners_label_3->setObjectName(QStringLiteral("wb_corners_label_3"));
        wb_corners_label_3->setAlignment(Qt::AlignCenter);

        WB_Navigation->addWidget(wb_corners_label_3);

        wb_corners_label_4 = new QLabel(horizontalLayoutWidget_2);
        wb_corners_label_4->setObjectName(QStringLiteral("wb_corners_label_4"));
        wb_corners_label_4->setFont(font1);
        wb_corners_label_4->setFrameShape(QFrame::NoFrame);
        wb_corners_label_4->setFrameShadow(QFrame::Raised);
        wb_corners_label_4->setAlignment(Qt::AlignCenter);

        WB_Navigation->addWidget(wb_corners_label_4);

        WBC_Continue_Button = new QPushButton(horizontalLayoutWidget_2);
        WBC_Continue_Button->setObjectName(QStringLiteral("WBC_Continue_Button"));

        WB_Navigation->addWidget(WBC_Continue_Button);

        horizontalLayoutWidget_3 = new QWidget(whiteboardCornersWidget);
        horizontalLayoutWidget_3->setObjectName(QStringLiteral("horizontalLayoutWidget_3"));
        horizontalLayoutWidget_3->setGeometry(QRect(10, 420, 1001, 31));
        WB_Controls = new QHBoxLayout(horizontalLayoutWidget_3);
        WB_Controls->setSpacing(6);
        WB_Controls->setContentsMargins(11, 11, 11, 11);
        WB_Controls->setObjectName(QStringLiteral("WB_Controls"));
        WB_Controls->setContentsMargins(0, 0, 0, 0);
        WBC_PrevWB = new QPushButton(horizontalLayoutWidget_3);
        WBC_PrevWB->setObjectName(QStringLiteral("WBC_PrevWB"));

        WB_Controls->addWidget(WBC_PrevWB);

        WBC_Clear = new QPushButton(horizontalLayoutWidget_3);
        WBC_Clear->setObjectName(QStringLiteral("WBC_Clear"));

        WB_Controls->addWidget(WBC_Clear);

        WBC_Save = new QPushButton(horizontalLayoutWidget_3);
        WBC_Save->setObjectName(QStringLiteral("WBC_Save"));

        WB_Controls->addWidget(WBC_Save);

        WBC_NextWB = new QPushButton(horizontalLayoutWidget_3);
        WBC_NextWB->setObjectName(QStringLiteral("WBC_NextWB"));

        WB_Controls->addWidget(WBC_NextWB);

        verticalLayoutWidget = new QWidget(whiteboardCornersWidget);
        verticalLayoutWidget->setObjectName(QStringLiteral("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(10, 30, 1001, 381));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        wbc_label = new clickable_label(verticalLayoutWidget);
        wbc_label->setObjectName(QStringLiteral("wbc_label"));
        wbc_label->setMouseTracking(true);
        wbc_label->setAlignment(Qt::AlignCenter);

        verticalLayout->addWidget(wbc_label);

        wbc_title = new QLabel(whiteboardCornersWidget);
        wbc_title->setObjectName(QStringLiteral("wbc_title"));
        wbc_title->setGeometry(QRect(360, 0, 261, 31));
        wbc_title->setFont(font2);
        lectureDetailsWidget = new QWidget(centralWidget);
        lectureDetailsWidget->setObjectName(QStringLiteral("lectureDetailsWidget"));
        lectureDetailsWidget->setGeometry(QRect(10, 5, 1020, 480));
        lectureDetails_label = new QLabel(lectureDetailsWidget);
        lectureDetails_label->setObjectName(QStringLiteral("lectureDetails_label"));
        lectureDetails_label->setGeometry(QRect(430, 10, 160, 18));
        lectureDetails_label->setFont(font2);
        lectureDetails_label->setAlignment(Qt::AlignCenter);
        horizontalLayoutWidget_4 = new QWidget(lectureDetailsWidget);
        horizontalLayoutWidget_4->setObjectName(QStringLiteral("horizontalLayoutWidget_4"));
        horizontalLayoutWidget_4->setGeometry(QRect(10, 450, 1001, 29));
        lectureDetails_Navigation = new QHBoxLayout(horizontalLayoutWidget_4);
        lectureDetails_Navigation->setSpacing(6);
        lectureDetails_Navigation->setContentsMargins(11, 11, 11, 11);
        lectureDetails_Navigation->setObjectName(QStringLiteral("lectureDetails_Navigation"));
        lectureDetails_Navigation->setContentsMargins(0, 0, 0, 0);
        lecDet_Previous_Button = new QPushButton(horizontalLayoutWidget_4);
        lecDet_Previous_Button->setObjectName(QStringLiteral("lecDet_Previous_Button"));

        lectureDetails_Navigation->addWidget(lecDet_Previous_Button);

        lec_details_label_1 = new QLabel(horizontalLayoutWidget_4);
        lec_details_label_1->setObjectName(QStringLiteral("lec_details_label_1"));
        lec_details_label_1->setFont(font3);
        lec_details_label_1->setAlignment(Qt::AlignCenter);

        lectureDetails_Navigation->addWidget(lec_details_label_1);

        lec_details_label_2 = new QLabel(horizontalLayoutWidget_4);
        lec_details_label_2->setObjectName(QStringLiteral("lec_details_label_2"));
        lec_details_label_2->setAlignment(Qt::AlignCenter);

        lectureDetails_Navigation->addWidget(lec_details_label_2);

        lec_details_label_3 = new QLabel(horizontalLayoutWidget_4);
        lec_details_label_3->setObjectName(QStringLiteral("lec_details_label_3"));
        lec_details_label_3->setFont(font);
        lec_details_label_3->setAlignment(Qt::AlignCenter);

        lectureDetails_Navigation->addWidget(lec_details_label_3);

        lec_details_label_4 = new QLabel(horizontalLayoutWidget_4);
        lec_details_label_4->setObjectName(QStringLiteral("lec_details_label_4"));
        lec_details_label_4->setFont(font1);
        lec_details_label_4->setFrameShape(QFrame::NoFrame);
        lec_details_label_4->setFrameShadow(QFrame::Raised);
        lec_details_label_4->setAlignment(Qt::AlignCenter);

        lectureDetails_Navigation->addWidget(lec_details_label_4);

        lecDet_Continue_Button = new QPushButton(horizontalLayoutWidget_4);
        lecDet_Continue_Button->setObjectName(QStringLiteral("lecDet_Continue_Button"));

        lectureDetails_Navigation->addWidget(lecDet_Continue_Button);

        label = new QLabel(lectureDetailsWidget);
        label->setObjectName(QStringLiteral("label"));
        label->setGeometry(QRect(260, 290, 510, 70));
        QFont font5;
        font5.setPointSize(12);
        label->setFont(font5);
        label->setTextFormat(Qt::AutoText);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        label_2 = new QLabel(lectureDetailsWidget);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setGeometry(QRect(450, 130, 120, 17));
        label_2->setAlignment(Qt::AlignCenter);
        newCourseLabel = new QLabel(lectureDetailsWidget);
        newCourseLabel->setObjectName(QStringLiteral("newCourseLabel"));
        newCourseLabel->setGeometry(QRect(450, 210, 120, 17));
        newCourseLabel->setAlignment(Qt::AlignCenter);
        lecDet_courses = new QComboBox(lectureDetailsWidget);
        lecDet_courses->setObjectName(QStringLiteral("lecDet_courses"));
        lecDet_courses->setGeometry(QRect(410, 150, 200, 27));
        new_course_textbox = new QLineEdit(lectureDetailsWidget);
        new_course_textbox->setObjectName(QStringLiteral("new_course_textbox"));
        new_course_textbox->setGeometry(QRect(410, 230, 201, 27));
        captureLectureWidget = new QWidget(centralWidget);
        captureLectureWidget->setObjectName(QStringLiteral("captureLectureWidget"));
        captureLectureWidget->setGeometry(QRect(10, 5, 1020, 480));
        label_4 = new QLabel(captureLectureWidget);
        label_4->setObjectName(QStringLiteral("label_4"));
        label_4->setGeometry(QRect(420, 10, 171, 31));
        label_4->setFont(font2);
        label_4->setAlignment(Qt::AlignCenter);
        horizontalLayoutWidget_5 = new QWidget(captureLectureWidget);
        horizontalLayoutWidget_5->setObjectName(QStringLiteral("horizontalLayoutWidget_5"));
        horizontalLayoutWidget_5->setGeometry(QRect(10, 450, 1001, 29));
        captureLecture_Navigation = new QHBoxLayout(horizontalLayoutWidget_5);
        captureLecture_Navigation->setSpacing(6);
        captureLecture_Navigation->setContentsMargins(11, 11, 11, 11);
        captureLecture_Navigation->setObjectName(QStringLiteral("captureLecture_Navigation"));
        captureLecture_Navigation->setContentsMargins(0, 0, 0, 0);
        capture_details_1 = new QLabel(horizontalLayoutWidget_5);
        capture_details_1->setObjectName(QStringLiteral("capture_details_1"));
        capture_details_1->setFont(font3);
        capture_details_1->setAlignment(Qt::AlignCenter);

        captureLecture_Navigation->addWidget(capture_details_1);

        capture_details_2 = new QLabel(horizontalLayoutWidget_5);
        capture_details_2->setObjectName(QStringLiteral("capture_details_2"));
        capture_details_2->setAlignment(Qt::AlignCenter);

        captureLecture_Navigation->addWidget(capture_details_2);

        capture_details_3 = new QLabel(horizontalLayoutWidget_5);
        capture_details_3->setObjectName(QStringLiteral("capture_details_3"));
        capture_details_3->setFont(font3);
        capture_details_3->setAlignment(Qt::AlignCenter);

        captureLecture_Navigation->addWidget(capture_details_3);

        capture_details_4 = new QLabel(horizontalLayoutWidget_5);
        capture_details_4->setObjectName(QStringLiteral("capture_details_4"));
        QFont font6;
        font6.setBold(true);
        font6.setUnderline(false);
        font6.setWeight(75);
        capture_details_4->setFont(font6);
        capture_details_4->setFrameShape(QFrame::NoFrame);
        capture_details_4->setFrameShadow(QFrame::Raised);
        capture_details_4->setAlignment(Qt::AlignCenter);

        captureLecture_Navigation->addWidget(capture_details_4);

        horizontalLayoutWidget_6 = new QWidget(captureLectureWidget);
        horizontalLayoutWidget_6->setObjectName(QStringLiteral("horizontalLayoutWidget_6"));
        horizontalLayoutWidget_6->setGeometry(QRect(10, 415, 1001, 31));
        captureLecture_Info = new QHBoxLayout(horizontalLayoutWidget_6);
        captureLecture_Info->setSpacing(6);
        captureLecture_Info->setContentsMargins(11, 11, 11, 11);
        captureLecture_Info->setObjectName(QStringLiteral("captureLecture_Info"));
        captureLecture_Info->setContentsMargins(0, 0, 0, 0);
        captureLecture_Timer = new QLabel(horizontalLayoutWidget_6);
        captureLecture_Timer->setObjectName(QStringLiteral("captureLecture_Timer"));

        captureLecture_Info->addWidget(captureLecture_Timer);

        captureLecture_Video_Status = new QLabel(horizontalLayoutWidget_6);
        captureLecture_Video_Status->setObjectName(QStringLiteral("captureLecture_Video_Status"));

        captureLecture_Info->addWidget(captureLecture_Video_Status);

        captureLecture_Terminate_Button = new QPushButton(horizontalLayoutWidget_6);
        captureLecture_Terminate_Button->setObjectName(QStringLiteral("captureLecture_Terminate_Button"));

        captureLecture_Info->addWidget(captureLecture_Terminate_Button);

        gridLayoutWidget_2 = new QWidget(captureLectureWidget);
        gridLayoutWidget_2->setObjectName(QStringLiteral("gridLayoutWidget_2"));
        gridLayoutWidget_2->setGeometry(QRect(10, 50, 1001, 361));
        lectureCaptureGrid = new QGridLayout(gridLayoutWidget_2);
        lectureCaptureGrid->setSpacing(6);
        lectureCaptureGrid->setContentsMargins(11, 11, 11, 11);
        lectureCaptureGrid->setObjectName(QStringLiteral("lectureCaptureGrid"));
        lectureCaptureGrid->setContentsMargins(0, 0, 0, 0);
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1037, 20));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
        mainMenu_Full_Run->setText(QApplication::translate("MainWindow", "Set up cameras and capture lecture", 0));
        mainMenu_Upload_Lectures->setText(QApplication::translate("MainWindow", "Upload lectures", 0));
        mainMenu_Setup_Cameras->setText(QApplication::translate("MainWindow", "Set up cameras", 0));
        setupReturnButton->setText(QApplication::translate("MainWindow", "Back to home", 0));
        device_setup_label->setText(QApplication::translate("MainWindow", "1. Device Setup", 0));
        wb_corners_label->setText(QApplication::translate("MainWindow", "2. Whiteboard Corners", 0));
        lec_details_label->setText(QApplication::translate("MainWindow", "3. Lecture Details", 0));
        cap_lec_label->setText(QApplication::translate("MainWindow", "4. Capture Lecture", 0));
        setupContinueButton->setText(QApplication::translate("MainWindow", "Next", 0));
        setup_titleLabel->setText(QApplication::translate("MainWindow", "Device Setup", 0));
        WBC_Return_Button->setText(QApplication::translate("MainWindow", "Return to Camera Setup", 0));
        wb_corners_label_1->setText(QApplication::translate("MainWindow", "1. Device Setup", 0));
        wb_corners_label_2->setText(QApplication::translate("MainWindow", "2. Whiteboard Corners", 0));
        wb_corners_label_3->setText(QApplication::translate("MainWindow", "3. Lecture Details", 0));
        wb_corners_label_4->setText(QApplication::translate("MainWindow", "4. Capture Lecture", 0));
        WBC_Continue_Button->setText(QApplication::translate("MainWindow", "Next", 0));
        WBC_PrevWB->setText(QApplication::translate("MainWindow", "Display Previous Whiteboard", 0));
        WBC_Clear->setText(QApplication::translate("MainWindow", "Clean Lines", 0));
        WBC_Save->setText(QApplication::translate("MainWindow", "Save Corners", 0));
        WBC_NextWB->setText(QApplication::translate("MainWindow", "Display Next Whiteboard", 0));
        wbc_label->setText(QApplication::translate("MainWindow", "Waiting for Image", 0));
        wbc_title->setText(QApplication::translate("MainWindow", "Whiteboard Corner Setup", 0));
        lectureDetails_label->setText(QApplication::translate("MainWindow", "Lecture Details", 0));
        lecDet_Previous_Button->setText(QApplication::translate("MainWindow", "Previous", 0));
        lec_details_label_1->setText(QApplication::translate("MainWindow", "1. Device Setup", 0));
        lec_details_label_2->setText(QApplication::translate("MainWindow", "2. Whiteboard Corners", 0));
        lec_details_label_3->setText(QApplication::translate("MainWindow", "3. Lecture Details", 0));
        lec_details_label_4->setText(QApplication::translate("MainWindow", "4. Capture Lecture", 0));
        lecDet_Continue_Button->setText(QApplication::translate("MainWindow", "Start Capture", 0));
        label->setText(QApplication::translate("MainWindow", "Instructions: Select your coutrse from the drop-down menu. If your course is not listed, choose \"Other\". A text box will appear and you can enter the new course name in there.", 0));
        label_2->setText(QApplication::translate("MainWindow", "Course", 0));
        newCourseLabel->setText(QApplication::translate("MainWindow", "New Course Title", 0));
        label_4->setText(QApplication::translate("MainWindow", "Lecture Capture", 0));
        capture_details_1->setText(QApplication::translate("MainWindow", "1. Device Setup", 0));
        capture_details_2->setText(QApplication::translate("MainWindow", "2. Whiteboard Corners", 0));
        capture_details_3->setText(QApplication::translate("MainWindow", "3. Lecture Details", 0));
        capture_details_4->setText(QApplication::translate("MainWindow", "4. Capture Lecture", 0));
        captureLecture_Timer->setText(QApplication::translate("MainWindow", "Time Elapsed:", 0));
        captureLecture_Video_Status->setText(QApplication::translate("MainWindow", "Video Status:", 0));
        captureLecture_Terminate_Button->setText(QApplication::translate("MainWindow", "Finish Capture", 0));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H

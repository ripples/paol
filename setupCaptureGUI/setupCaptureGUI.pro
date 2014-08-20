#-------------------------------------------------
#
# Project created by QtCreator 2014-08-09T15:14:36
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = setupCaptureGUI
TEMPLATE = app


SOURCES += main.cpp\
        mainWindow.cpp \
    paolMat.cpp

HEADERS  += mainWindow.h \
    paolMat.h

FORMS    += mainWindow.ui

LIBS += `pkg-config opencv --libs`


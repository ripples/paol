#-------------------------------------------------
#
# Project created by QtCreator 2014-06-01T19:04:58
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = setupCaptureGUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    paolMat.cpp

HEADERS  += mainwindow.h \
    paolMat.h

FORMS    += mainwindow.ui

LIBS += `pkg-config opencv --libs`

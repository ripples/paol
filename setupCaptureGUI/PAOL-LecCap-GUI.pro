#-------------------------------------------------
#
# Project created by QtCreator 2015-01-07T10:19:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = PAOL-LecCap-GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainWindow.cpp \
        paolMat.cpp \
        paolProcess.cpp \
    uf.cpp \
    WhiteboardProcessor.cpp

HEADERS  += mainWindow.h \
        paolMat.h \
        paolProcess.h \
    uf.h \
    WhiteboardProcessor.h

FORMS    += mainWindow.ui

LIBS += `pkg-config opencv --libs`

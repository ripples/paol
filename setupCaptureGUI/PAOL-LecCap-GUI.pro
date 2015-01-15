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
        paolProcess.cpp \
    uf.cpp \
    WhiteboardProcessor.cpp \
    vgaprocess.cpp \
    whiteboardprocess.cpp

HEADERS  += mainWindow.h \
        paolProcess.h \
    uf.h \
    WhiteboardProcessor.h \
    vgaprocess.h \
    whiteboardprocess.h

FORMS    += mainWindow.ui

LIBS += `pkg-config opencv --libs`

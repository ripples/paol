#include "mainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Required to be able to signal with Mats
    qRegisterMetaType<Mat>("Mat");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

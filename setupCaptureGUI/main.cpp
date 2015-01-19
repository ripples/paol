#include "mainWindow.h"
#include "commandlinethread.h"
#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QTimer>

int main(int argc, char *argv[])
{
    // Required to be able to signal with Mats
    qRegisterMetaType<Mat>("Mat");

//    QCoreApplication core(argc, argv);
//    CommandLineThread* cmdThread = new CommandLineThread(&core);
//    QObject::connect(cmdThread, SIGNAL(finished()), &core, SLOT(quit()));
//    QTimer::singleShot(0, cmdThread, SLOT(run()));
//    return core.exec();

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}

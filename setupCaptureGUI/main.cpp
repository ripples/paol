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

    if(argc == 4) {
        // Run the command-line version if the correct number of arguments were detected
        CommandLineThread* cmdThread = new CommandLineThread(argc, argv);
        cmdThread->start();
        cmdThread->wait();
        return 0;
    }
    else {
        // Run the GUI application
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();
    }
}

#include "mainWindow.h"
#include "commandlinethread.h"
#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QTimer>
#include <QApplication>

// Redirect Qt output to standard output. Modified from
// http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
void paolMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    fprintf(stdout, "%s\n", localMsg.constData());
}

int main(int argc, char *argv[])
{
    // Required to be able to signal with Mats
    qRegisterMetaType<Mat>("Mat");

    if(argc == 4) {
        // Run the command-line version if the correct number of arguments were detected
        QCoreApplication a(argc, argv);
        // Redirect Qt debugging output to standard output
        qInstallMessageHandler(paolMessageHandler);
        CommandLineThread* cmdThread = new CommandLineThread(argc, argv);
        QObject::connect(cmdThread, SIGNAL(finished()), &a, SLOT(quit()));
        QTimer::singleShot(0, cmdThread, SLOT(run()));
        return a.exec();
    }
    else {
        // Run the GUI application
        QApplication a(argc, argv);
        MainWindow w;
        w.show();

        return a.exec();
    }
}

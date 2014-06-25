#include "windowCreator.h"

class WCwindow : public QMainWindow{

public:
    WCwindow();

private:
    QVector<QLabel*> cams;
    QLabel* cam(int r, int c) const {
        return cams[r*3 + c];
    }
};

WCwindow::WCwindow()
{
    QGridLayout *layout = new QGridLayout;

    for (int i = 1; i < 10; ++ i) {
        QLabel * const label = new QLabel(QString("CAM %1").arg(i, 2, 10, QLatin1Char('0')));
        label->setFixedSize(200, 50);
        layout->addWidget(label, (i-1) / 3, (i-1) % 3);
        cams << label;
    }

    QWidget * central = new QWidget();
    setCentralWidget(central);
    centralWidget()->setLayout(layout);

    setWindowTitle("Camera Window");
    setFixedSize(1000, 800);
}

windowCreator::windowCreator(){
    WCwindow win;
    win.show();
}

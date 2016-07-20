#ifndef WORKER_H
#define WORKER_H

#include <QtCore>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

class Worker : public QObject
{
    Q_OBJECT

protected:
    // Webcam and associated properties
    VideoCapture camera;
    bool flipCam;
    int deviceNum;
    bool runningGUI;

    // Field to store where the lecture is
    string lecturePath;
    FILE* logFile;

    // Fields for general processing (mostly saving the image)
    int saveImageCount;
    time_t currentImageTime;
    // How many images were captured from the camera
    int capturedImageCount;
    // Flag for whether a non-empty image or model is stored. This is
    // used to avoid saving the first stable image, which will just
    // be a black frame.
    bool realImageIsStored;

    void writeFinishStatistics();

    virtual bool takePicture() = 0;
    virtual void processImage() = 0;
    virtual void saveImageWithTimestamp(const Mat& image) = 0;
    virtual void printToLog(char* format, ...) = 0;
    virtual void saveLastImage() = 0;

public:
    Worker();
    ~Worker();
    void finish();

private slots:
    void workOnNextImage();

signals:
    void savedImage(const Mat& image);
    void capturedImage(Mat image);
};

#endif // WORKER_H

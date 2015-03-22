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
    Mat currentFrame;

    // Webcam and associated properties
    VideoCapture camera;
    bool flipCam;
    int deviceNum;

    // Field to store where the lecture is
    string lecturePath;
    FILE* logFile;

    // Fields for general processing (mostly saving the image)
    int saveImageCount;
    time_t currentImageTime;
    int capturedImageCount; // How many images were captured from the camera

    virtual bool takePicture() = 0;
    virtual void processImage() = 0;
    virtual void saveImageWithTimestamp(const Mat& image) = 0;
    virtual void printToLog(char* format, ...) = 0;

public:
    ~Worker();
    void writeFinishStatistics();

private slots:
    void workOnNextImage();

signals:
    void savedImage(const Mat& image);
    void capturedImage(Mat image);
};

#endif // WORKER_H

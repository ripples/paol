#ifndef SEGLIST_H
#define SEGLIST_H

#include <vector>
#include <QtCore>
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

class SegList
{
public:
    SegList();
    ~SegList();
    int find(int x);
    int merge(int u,int l);
    int newPoint();
    void update();
    void print();
    int getCount();
    int getValue(int x);
    void zero();
    void addCount(int loc, Mat &im, int x, int y);
    void removeSmallRegions(int thresh);
    void removeNoneWhite(int thresh);
    void removeColor(int color);

private:
    int ccount;
    vector<int> sList;
    vector<int> pixCount;
    vector<double> red;
    vector<double> green;
    vector<double> blue;
    int rcount;
};

#endif // SEGLIST_H

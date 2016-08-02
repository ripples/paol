#ifndef PAOLPROCUTILS_H
#define PAOLPROCUTILS_H

#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include <opencv2/highgui/highgui.hpp>
#include <QLabel>
#include <QtCore>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <QString>

#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdexcept>
#include "uf.h"
#include "seglist.h"

using namespace cv;

// Structure for defining a set of whiteboard corners. The coordinates follow the ordering
// for images, i.e. the x-coordinate is the horizontal axis, and the y-coordinate is the
// vertical axis.
typedef struct wbc WBCorners;
struct wbc {
    Point2f TL, TR, BL, BR;
};

class PAOLProcUtils
{
public:
    /// Constants for whiteboard processing
    // Scaling factor for processing on smaller versions of whiteboard
    static const int SCALE = 8;
    // Expected upper bound on how many connected components are found in a DoG image
    static const int DEFAULT_NUM_CC = 50000;

    /// Consts for VGA processing
    static const int BOTTOM_MASK = 115;
    static const float DIFF_THRESHOLD = 0.0002;
    static const int REPEAT = 3;
    static const int PIXEL_DIFF_THRESHOLD = 100;
    static const int BORDER_DIFF_THRESHOLD = 50;
    static const int WINDOW_SIZE = 0;

    /// Utility methods

    /// Methods to find and process differences (ie. find the lecturer)
    static Mat replicateToImageBorder(const Mat& orig);
    static Mat sweepDown(const Mat& orig);
    static Mat borderContentWithGreen(const Mat& content, int borderSize);
    static Mat grow(const Mat& orig, int size);
    static Mat growGreen(const Mat& orig, int size);
    static Mat getImageContours(const Mat& orig);
    static void findAllDiffsMini(Mat& diffLocations, float& percentDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size);
    static void filterNoisyDiffs(Mat& filteredDiffs, float& percentDiff, const Mat& origDiffs);
    static Mat enlarge(const Mat& orig);
    static Mat expandDifferencesRegion(const Mat& differences);
    static Mat difference(const Mat& old,const Mat& current,int threshold);
    static Mat NCC(const Mat& old, const Mat& current, float threshold, int size);
    static void StablePix(Mat &stableIm, const Mat& old, const Mat& current, int thresh);
    static void updateBackground(Mat &background, const Mat& current, const Mat& stable, int time, const Mat &refined);

    /// Methods to find the marker strokes
    static Mat binarizeAnd(const Mat& orig, int threshold);
    static Mat binarizeOr(const Mat& orig, int threshold);
    static Mat binarize(const Mat& orig, int threshold);
    static Mat thresholdOnBlueChannel(const Mat& orig, int blueThresh, int size);
    static Mat pDrift(const Mat& orig);
    static Mat fillMarkerBorders(const Mat& markerBorders);
    static Mat getDoGEdges(const Mat& orig, int kerSize, float rad1, float rad2);
    static Mat adjustLevels(const Mat& orig, int lo, int hi, double gamma);
    static int** getConnectedComponents(const Mat& components);
    static Mat filterConnectedComponents(const Mat& compsImg, const Mat& keepCompLocs);
    static Mat findMarkerWithMarkerBorders(const Mat& whiteboardImage);
    static Mat findMarkerWithCC(const Mat& orig);
    static Mat findMarkerStrokeCandidates(const Mat& orig);
    static Mat findMarkerStrokeLocations(const Mat& orig);
    static float findMarkerModelDiffs(const Mat& oldMarkerModel, const Mat& newMarkerModel);
    static float findMarkerStrokeDiffs(const Mat& oldMarkerModel, const Mat& newMarkerModel);
    static Mat findMarkerStrokeDiffs2(const Mat& oldMarkerModel, const Mat& newMarkerModel);
    static Mat getNotWhite(const Mat& whiteboard);

    static Mat CLAHE(const Mat &orig);
    static Mat darkenText(Mat &pdrift, const Mat& orig);
    static Mat enhanceColor(const Mat &orig);

    /// Methods to enhance the whiteboard
    static Mat boxBlur(const Mat& orig, int size);
    static Mat getAvgWhiteboardColor(const Mat& whiteboardImg, int size);
    static Mat raiseMarkerContrast(const Mat& whiteboardImg);
    static Mat refineImage(const Mat& current, const Mat& avg);
    static Mat whitenWhiteboard(const Mat &whiteboardImg, const Mat& markerPixels);
    static Mat rectifyImage(const Mat& whiteboardImg, const WBCorners& corners);
    static Mat findWhiteboardBorders(Mat& whiteboardImg);
    static Mat smoothMarkerTransition(const Mat& whiteWhiteboardImage);
    static int countNoneWhite(const Mat& background);
    static Mat erodeSize(const Mat& mask,int size);
    static Mat erodeSizeGreen(const Mat& mask,int size);
    static int countDifferences(const Mat& difs);

    /// Update the background (whiteboard) model
    static Mat updateModel(const Mat& oldModel, const Mat& newInfo, const Mat& oldInfoMask);

    /// Method to find differences between computer images
    static float getVGADifferences(const Mat& oldFrame, const Mat& newFrame);
    //whiteboard difference test
    static Mat getWhiteboardDifferences(const Mat& oldFrame, const Mat& newFrame);
    static Mat getErodeDifferencesIm(const Mat& oldFrame, const Mat& newFrame);
    static int getErodeDifferencesNum(const Mat& oldFrame, const Mat& newFrame);

    /// Method to sort whiteboard corners into TL, TR, BR, BL
    static void sortCorners(WBCorners& corners);

    ///background subtraction ideas
    static Mat surrountDifference(Mat& aveIm);

    ///connected component code
    static Mat connectedComponent(Mat& im, bool binary);
    static Mat connectedComponentFlipEliminate(Mat& im, Mat &orig, bool binary);
    static void colorConnected(Mat& connected, SegList *sList);
    static int comparePix(Mat& connected, Mat &data, int x1, int y1, int x2, int y2, bool binary, int thresh, SegList *sList);
    static bool pixSame(Mat& im,int x1,int y1,int x2,int y2,int thresh);
    static int pixToNum(Mat& im,int x,int y);
    static void numToPix(Mat& im,int x,int y,int num);
    static void numToPix2(Mat& im,Mat& orig,int x,int y,int num);
    static Mat minimalDif(Mat& im,int thresh);
    static Mat invertToBinary(Mat &im);
    static Mat keepWhite(Mat &im, int thresh);
};

#endif // PAOLPROCUTILS_H

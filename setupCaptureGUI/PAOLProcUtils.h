#ifndef PAOLPROCUTILS_H
#define PAOLPROCUTILS_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdexcept>
#include "uf.h"

using namespace cv;

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
    static const float DIFF_THRESHOLD = .0002;
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
    static Mat getImageContours(const Mat& orig);
    static void findAllDiffsMini(Mat& diffLocations, float& percentDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size);
    static void filterNoisyDiffs(Mat& filteredDiffs, float& percentDiff, const Mat& origDiffs);
    static Mat enlarge(const Mat& orig);
    static Mat expandDifferencesRegion(const Mat& differences);

    /// Methods to find the marker strokes
    static Mat binarizeAnd(const Mat& orig, int threshold);
    static Mat binarizeOr(const Mat& orig, int threshold);
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

    /// Methods to enhance the whiteboard
    static Mat boxBlur(const Mat& orig, int size);
    static Mat getAvgWhiteboardColor(const Mat& whiteboardImg, int size);
    static Mat raiseMarkerContrast(const Mat& whiteboardImg);
    static Mat whitenWhiteboard(const Mat &whiteboardImg, const Mat& markerPixels);
    static Mat rectifyImage(const Mat& whiteboardImg, const WBCorners& corners);
    static Mat findWhiteboardBorders(Mat& whiteboardImg);
    static Mat smoothMarkerTransition(const Mat& whiteWhiteboardImage);

    /// Update the background (whiteboard) model
    static Mat updateModel(const Mat& oldModel, const Mat& newInfo, const Mat& oldInfoMask);

    /// Method to find differences between computer images
    static float getVGADifferences(const Mat& oldFrame, const Mat& newFrame);

    /// Method to sort whiteboard corners into TL, TR, BR, BL
    static void sortCorners(WBCorners& corners);
};

#endif // PAOLPROCUTILS_H

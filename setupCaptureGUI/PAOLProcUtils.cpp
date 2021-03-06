#include "PAOLProcUtils.h"

const float PAOLProcUtils::DIFF_THRESHOLD = 0.0002;

///////////////////////////////////////////////////////////////////
///
///   Methods to find and process differences (ie. find the lecturer)
///
///////////////////////////////////////////////////////////////////

// Shrink the two given images, then find the significantly different pixels between the shrunken images
// For a pixel to be considered significantly different, one of the channels must differ by more than
// the threshold between the two images
// Arguments:
//    diffLocations: Where to store the differences from the shrunken images
//    percentDiff: Where to store the percentage of pixels that differ
//    oldImg: The previous (older) whiteboard image
//    newImg: The current (newer) whiteboard image
//    thresh: How much a channel needs to differ between a pixel to count the pixel as a difference
//    size: Size of window to use to filter out differences near edges
void PAOLProcUtils::findAllDiffsMini(Mat& diffLocations, float& percentDiff, const Mat& oldImg, const Mat& newImg, int thresh, int size) {
    int offset;
    bool diff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    diffLocations=Mat::zeros(oldImg.rows/SCALE,oldImg.cols/SCALE,oldImg.type());

    if (SCALE>=size+1)
        offset=SCALE;
    else
        offset=SCALE*2;

  percentDiff = 0;

  for (int y = offset, yy=offset/SCALE; y < (oldImg.rows-offset); y+=SCALE,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/SCALE; x < (oldImg.cols-offset); x+=SCALE,xx++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)newImg.at<Vec3b>(y,x)[i]-(double)oldImg.at<Vec3b>(y,x)[i]) > thresh)
                  diff = true;
          }
          if(diff)
          {
              for(int yy = y-size; yy <= y+size; yy++)
              {
                  for(int xx = x-size; xx <= x+size; xx++)
                  {
                      //for each color channel
                      for(int ii = 0; ii < 3; ii++)
                      {
                          //ignore all differneces found at the edges; sometimes pixels get lost in tranmission
                          if(abs(((double)(newImg.at<Vec3b>(yy,xx)[ii]))-(((double)(newImg.at<Vec3b>((yy+1),xx)[ii])))>surroundThresh))
                              diff = false;
                          if(abs(((double)(newImg.at<Vec3b>(yy,xx)[ii]))-(((double)(newImg.at<Vec3b>(yy,(xx+1))[ii])))>surroundThresh))
                              diff = false;
                      }
                  }
              }
          }
          if(diff)
          {
              percentDiff++;
              diffLocations.at<Vec3b>(yy,xx)[2]=255;
          }
      }
  }
  //return the percent of maskMin that are differences
  percentDiff = (float)percentDiff/(float)(diffLocations.rows*diffLocations.cols);
}

// Filter out pixels that are not surrounded by enough difference pixels
// Arguments:
//    filteredDiffs: Where to store the pixels that are surrounded by enough difference pixels
//    percentDiff: Where to store the percentage of white pixels in filteredDiffs
//    origDiffs: The unfiltered difference pixels (gotten from findAllDiffsMini)
void PAOLProcUtils::filterNoisyDiffs(Mat& filteredDiffs, float& percentDiff, const Mat& origDiffs) {
    filteredDiffs = Mat::zeros(origDiffs.size(), origDiffs.type());
    int total;
    percentDiff = 0;
    //for every 3x3 group of pixels in maskMin
    for(int x = 1; x < origDiffs.cols-1; x++)
        for(int y = 1; y < origDiffs.rows-1; y++)
        {
            //count the number of differences in the 3x3
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(origDiffs.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            //if the number of differences is greater then 3 mark it as different and count it
            if(total>3){
                filteredDiffs.at<Vec3b>(y,x)[1]=255;
                percentDiff++;
            }
        }
    //return the percentage of the maskMin that are valid differences
    percentDiff = (float)percentDiff/(float)(origDiffs.rows*origDiffs.cols);
}

// Given a binary image, fill the image border by replicating the adjacent pixel
Mat PAOLProcUtils::replicateToImageBorder(const Mat& orig) {
    Mat origWithBorderPixels = orig.clone();
    int x;

    //extend bottom and top edge where differences overlap
    for(x=0;x<origWithBorderPixels.cols;x++){
        if(origWithBorderPixels.at<Vec3b>(1,x)[1] == 255)
            origWithBorderPixels.at<Vec3b>(0,x)[1]=255;
        if(origWithBorderPixels.at<Vec3b>(origWithBorderPixels.rows-2,x)[1] == 255)
            origWithBorderPixels.at<Vec3b>(origWithBorderPixels.rows-1,x)[1]=255;
    }

    //extend right and left edge wher differences overlap
    for(int y = 0; y < origWithBorderPixels.rows; y++)
    {
        if(origWithBorderPixels.at<Vec3b>(y,1)[1] == 255)
            origWithBorderPixels.at<Vec3b>(y,0)[1] = 255;

        if(origWithBorderPixels.at<Vec3b>(y,origWithBorderPixels.cols-2)[1] == 255)
            origWithBorderPixels.at<Vec3b>(y,origWithBorderPixels.cols-1)[1] = 255;
    }
    return origWithBorderPixels;
}

// Sweep the given image (whatever that means), returns a binary image
Mat PAOLProcUtils::sweepDown(const Mat& orig){
    bool left,right,top;
    //create a Mat the size of orig to store results
    Mat sweeped=Mat::zeros(orig.size(),orig.type());

    //from left to right
    for(int x = 0; x < orig.cols; x++)
    {
        //from top to bottom
        top = false;
        for(int y = 0; y < orig.rows; y++){
            if(orig.at<Vec3b>(y,x)[1] == 255)
                top = true;

            if(top == true)
                sweeped.at<Vec3b>(y,x)[0] = 255;
        }
    }

    //from top to bottom
    for(int y = 0; y < orig.rows; y++){

        //sweep from the left
        left = false;
        for(int x = 0; x < orig.cols; x++)
        {
            if(orig.at<Vec3b>(y,x)[1] == 255)
                left = true;

            if(left == true)
                sweeped.at<Vec3b>(y,x)[1] = 255;
        }

        //sweep from the right
        right = false;
        for(int x = orig.cols-1; x >-1; x--)
        {
            if(orig.at<Vec3b>(y,x)[1] == 255)
                right = true;

            if(right == true)
                sweeped.at<Vec3b>(y,x)[2] = 255;
        }
    }

    return sweeped;
}

// Color the region around the white pixels of the given image with green. This
// is helpful for debugging.
// Arguments:
//    content: A binary image with white pixels to place the border around
//    borderSize: The thickness of the border
Mat PAOLProcUtils::borderContentWithGreen(const Mat& content, int borderSize)
{
    Mat ret = content.clone();
    int startx,endx,starty,endy;

    //for every pixel in the image
    for(int y = 0; y < ret.rows; y++)
        for(int x = 0; x < ret.cols; x++)

            //if the pixel is turned on
            if(ret.at<Vec3b>(y,x)[0] == 255){
                startx=x-borderSize;
                if (startx<0)
                    startx=0;

                starty=y-borderSize;
                if (starty<0)
                    starty=0;

                endx=x+borderSize;
                if (endx>=ret.cols)
                    endx=ret.cols-1;

                endy=y+borderSize;
                if (endy>=ret.rows)
                    endy=ret.rows-1;

                //grow the region around that pixel
                for(int yy = starty; yy <= endy;yy++)
                    for(int xx = startx; xx <= endx; xx++)
                        ret.at<Vec3b>(yy,xx)[1] = 255;
            }
    return ret;
}

// Dilate the given image by the given size
Mat PAOLProcUtils::grow(const Mat& orig, int size) {
    Mat ret = orig.clone();
    Mat element = getStructuringElement(MORPH_RECT, Size(2*size+1,2*size+1));
    dilate(orig, ret, element);
    element.release();
    return ret;
}

// Dilate the given image by the given size
Mat PAOLProcUtils::growGreen(const Mat& orig, int size) {
    Mat ret = orig.clone();
    Mat element = getStructuringElement(MORPH_RECT, Size(2*size+1,2*size+1));
    dilate(orig, ret, element);
    for(int y = 0; y < ret.rows; y++)
        for(int x = 0; x < ret.cols; x++)

            //if the pixel is turned on
            if(ret.at<Vec3b>(y,x)[0] == 255 && orig.at<Vec3b>(y,x)[0] == 0){
                ret.at<Vec3b>(y,x)[1]=0;
            }
    element.release();
    return ret;
}

// Get the contours of the given image
Mat PAOLProcUtils::getImageContours(const Mat& orig) {
    Mat contourImage = orig.clone();
    Mat src_gray;
    int thresh = 100;
    //int max_thresh = 255;
    RNG rng(12345);

    /// Convert image to gray and blur it
    cvtColor( orig, src_gray, CV_BGR2GRAY );
    cv::blur( src_gray, src_gray, cv::Size(3,3) );

    Mat canny_output;
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;

    /// Detect edges using canny
    Canny( src_gray, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

    vector<vector<Point> >hull( contours.size() );
    for(unsigned int i = 0; i < contours.size(); i++ )
    {  convexHull( Mat(contours[i]), hull[i], false ); }

    /// Draw contours + hull results
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    for(unsigned int i = 0; i< contours.size(); i++ )
    {
        Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
        drawContours( drawing, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        drawContours( drawing, hull, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    }

    int count;
    //add the contours to maskmin
    for(int x = 0; x < orig.cols; x++)
      for(int y = 0; y < orig.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=drawing.at<Vec3b>(y,x)[c];
          if(count>0)
              contourImage.at<Vec3b>(y,x)[1]=255;
      }
    src_gray.release();
    drawing.release();
    canny_output.release();
    return contourImage;
}

// Enlarge the given image by a factor of WhiteboardProcessor::SCALE
Mat PAOLProcUtils::enlarge(const Mat& orig) {
    Mat enlarged = Mat::zeros(Size(orig.cols*SCALE, orig.rows*SCALE), orig.type());
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < enlarged.rows; y+=SCALE)
        {
            for (int x = 0; x < enlarged.cols; x+=SCALE)
            {
                //set the location on if the corresponding location in maskMin is on
                if (orig.at<Vec3b>(y/SCALE,x/SCALE)[c]!=0)
                    center=true;
                else
                    center=false;

                //repeat for other corners of box
                if((x+SCALE)/SCALE<orig.cols){
                    rightIn=true;
                    if(orig.at<Vec3b>(y/SCALE,(x+SCALE)/SCALE)[c]!=0)
                        right=true;
                    else
                        right=false;
                } else
                    rightIn=false;

                if((y+SCALE)/SCALE<orig.rows){
                    downIn=true;
                    if(orig.at<Vec3b>((y+SCALE)/SCALE,x/SCALE)[c]!=0)
                        down=true;
                    else
                        down=false;
                } else
                    downIn=false;

                if(downIn && rightIn){
                    //cornerIn=true;
                    if(orig.at<Vec3b>((y+SCALE)/SCALE,(x+SCALE)/SCALE)[c]!=0)
                        corner=true;
                    else
                        corner=false;
                } //else
                    //cornerIn=false;

                //fill in mask based on which corners are turned on based on maskMin
                if(center)
                    enlarged.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<enlarged.cols && xx<x+SCALE;xx++)
                        enlarged.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<enlarged.rows && yy<y+SCALE; yy++)
                        enlarged.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<enlarged.cols && xx<x+SCALE;xx++)
                        for(int yy=y+1; yy<enlarged.rows && yy<y+SCALE; yy++)
                            enlarged.at<Vec3b>(yy,xx)[c]=255;

            }
        }
    }
    return enlarged;
}

// Covers the (filtered) difference pixels by first dilating the difference,
// then drawing a convex hull around the dilation
Mat PAOLProcUtils::expandDifferencesRegion(const Mat& differences) {
    // Dilate the difference pixels
    Mat grownDiffs = replicateToImageBorder(differences);
    grownDiffs = sweepDown(grownDiffs);
    grownDiffs = binarizeAnd(grownDiffs, 255);
    grownDiffs = borderContentWithGreen(grownDiffs, 8);

    // Draw hull around the dilation
    Mat diffHulls = getImageContours(grownDiffs);
    diffHulls = sweepDown(diffHulls);
    diffHulls = binarizeAnd(diffHulls, 255);

    grownDiffs.release();
    return diffHulls;
}

Mat PAOLProcUtils::difference(const Mat& old,const Mat& current,int threshold){
    Mat out=Mat::zeros(current.size(), current.type());
    int count;

    for(int y = 0; y < current.rows; y++) {
        for(int x = 0; x < current.cols; x++) {
            count=0;
            for (int c=0;c<3;c++){
                count+=abs(old.at<Vec3b>(y,x)[c]-current.at<Vec3b>(y,x)[c]);
            }
            if (count>threshold){
                for (int c=0;c<3;c++){
                    out.at<Vec3b>(y,x)[c]=255;
                }
            }
        }
    }
    return out;
}

Mat PAOLProcUtils::NCC(const Mat& old,const Mat& current,float threshold,int size){
    Mat out=Mat::zeros(current.size(), current.type());
    double c[3];
    double N,stdev[3],stdev1[3],var[3],var1[3],Iave[3],Iave1[3],sum[3],ctotal,stdave;

    for(int y = 0; y < current.rows; y+=size) {
        for(int x = 0; x < current.cols; x+=size) {
            for (int ch=0;ch<3;ch++){
                c[ch]=0;
                stdev[ch]=0;
                stdev1[ch]=0;
                var[ch]=0;
                var1[ch]=0;
                Iave[ch]=0;
                Iave1[ch]=0;
                sum[ch]=0;
            }
            N=0;
            //calculate average
            for(int yy=y; yy<y+size && yy<current.rows; yy++)
                for(int xx=x; xx<x+size && xx<current.cols; xx++){
                    N++;
                    for(int ch=0;ch<3;ch++){
                        Iave[ch]+=old.at<Vec3b>(yy,xx)[ch];
                        Iave1[ch]+=current.at<Vec3b>(yy,xx)[ch];
                    }
                }
            for(int ch=0;ch<3;ch++){
                Iave[ch]/=N;
                Iave1[ch]/=N;
            }
            //calculate variance
            for(int yy=y; yy<y+size && yy<current.rows; yy++)
                for(int xx=x; xx<x+size && xx<current.cols; xx++){
                    for(int ch=0;ch<3;ch++){
                        var[ch]+=((double)old.at<Vec3b>(yy,xx)[ch]-Iave[ch])*((double)old.at<Vec3b>(yy,xx)[ch]-Iave[ch]);
                        var1[ch]+=((double)current.at<Vec3b>(yy,xx)[ch]-Iave1[ch])*((double)current.at<Vec3b>(yy,xx)[ch]-Iave1[ch]);
                    }
                }
            for(int ch=0;ch<3;ch++){
                var[ch]/=N;
                var1[ch]/=N;
            }
            //calculate std deviation
            stdave=0;
            for(int ch=0;ch<3;ch++){
                stdev[ch]=sqrt(var[ch]);
                stdev1[ch]=sqrt(var1[ch]);
                stdave+=stdev[ch]+stdev1[ch];
            }
            stdave/=6;

            //calculate normalized cross correlation
            for(int yy=y; yy<y+size && yy<current.rows; yy++)
                for(int xx=x; xx<x+size && xx<current.cols; xx++){
                    for(int ch=0;ch<3;ch++){
                        sum[ch]+=((double)old.at<Vec3b>(yy,xx)[ch]-Iave[ch])*((double)current.at<Vec3b>(yy,xx)[ch]-Iave1[ch]);
                    }
                }
            ctotal=0;
            for(int ch=0;ch<3;ch++){
                c[ch]=sum[ch]/(N*stdev[ch]*stdev1[ch]);
                ctotal+=c[ch];
            }
            ctotal/=3;
            if(ctotal<0)
                ctotal=-1;
            stdave*=10;
            if(stdave>255)
                stdave=255;
            //set output
            for(int yy=y; yy<y+size && yy<current.rows; yy++)
                for(int xx=x; xx<x+size && xx<current.cols; xx++){
                    //if(ctotal>=threshold){
                        //qDebug("%f ",ctotal);
                        out.at<Vec3b>(yy,xx)[1]=(int)(ctotal*255);
                    //}
                    if(stdave<20){
                        out.at<Vec3b>(yy,xx)[0]=(int)(255);
                    }
                    if(stdave>30 && ctotal<threshold){
                        out.at<Vec3b>(yy,xx)[2]=(int)(255);
                    }
                    //out.at<Vec3b>(yy,xx)[2]=(old.at<Vec3b>(yy,xx)[0]+old.at<Vec3b>(yy,xx)[1]+old.at<Vec3b>(yy,xx)[2])/3;
                    //out.at<Vec3b>(yy,xx)[0]=(current.at<Vec3b>(yy,xx)[0]+current.at<Vec3b>(yy,xx)[1]+current.at<Vec3b>(yy,xx)[2])/3;
                }
        }
    }
    return out;
}

void PAOLProcUtils::StablePix(Mat& stableIm,const Mat& old, const Mat& current,int thresh){
    for(int y = 0; y < current.rows; y++) {
        for(int x = 0; x < current.cols; x++) {
            if(abs(old.at<Vec3b>(y,x)[1]-current.at<Vec3b>(y,x)[1])<thresh){
                if(stableIm.at<Vec3b>(y,x)[1]<255){
                    stableIm.at<Vec3b>(y,x)[1]++;
                }
            } else {
                stableIm.at<Vec3b>(y,x)[1]=0;
            }
            if(abs(old.at<Vec3b>(y,x)[0]-current.at<Vec3b>(y,x)[0])<thresh &&
                    old.at<Vec3b>(y,x)[0]!=0 &&
                    current.at<Vec3b>(y,x)[0]){
                if(stableIm.at<Vec3b>(y,x)[0]<255){
                    stableIm.at<Vec3b>(y,x)[0]++;
                }
            } else{
                stableIm.at<Vec3b>(y,x)[0]=0;
            }
            if(old.at<Vec3b>(y,x)[2]!=0 || current.at<Vec3b>(y,x)[2]!=0){
                stableIm.at<Vec3b>(y,x)[2]=255;
                stableIm.at<Vec3b>(y,x)[0]=0;
                stableIm.at<Vec3b>(y,x)[1]=0;

            } else {
                stableIm.at<Vec3b>(y,x)[2]=0;
            }
            //stableIm.at<Vec3b>(y,x)[2]=stableIm.at<Vec3b>(y,x)[1]*5;
        }
    }
}

void PAOLProcUtils::updateBackground(Mat &background, const Mat &current, const Mat &stable, int time,const Mat& refined){
    int r,g,b,br,rg,gb;
    int thresh2=20;

    for(int y = 0; y < current.rows; y++) {
        for(int x = 0; x < current.cols; x++) {
            if(stable.at<Vec3b>(y,x)[1]>time){
                for (int c=0;c<3;c++){
                    background.at<Vec3b>(y,x)[c]=refined.at<Vec3b>(y,x)[c];
                }
            }
            if(stable.at<Vec3b>(y,x)[0]>time){
                r=current.at<Vec3b>(y,x)[2];
                g=current.at<Vec3b>(y,x)[1];
                b=current.at<Vec3b>(y,x)[0];

                br=abs(r-b);
                rg=abs(r-g);
                gb=abs(g-b);
                if(br<thresh2 &&
                        rg<thresh2 &&
                        gb<thresh2 &&
                        r>30){
                    for (int c=0;c<3;c++){
                        background.at<Vec3b>(y,x)[c]=255;//refined.at<Vec3b>(y,x)[c];
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////
///
///   Methods to find the marker strokes
///
///////////////////////////////////////////////////////////////////

// Determine the pixels in orig where all the values of all three
// channels are greater than the threshold
Mat PAOLProcUtils::binarizeAnd(const Mat& orig, int threshold) {
    Mat binarized = Mat::zeros(orig.size(), orig.type());
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            if(orig.at<Vec3b>(i,j)[0] >= threshold &&
                    orig.at<Vec3b>(i,j)[1] >= threshold &&
                    orig.at<Vec3b>(i,j)[2] >= threshold) {
                binarized.at<Vec3b>(i,j)[0] = 255;
                binarized.at<Vec3b>(i,j)[1] = 255;
                binarized.at<Vec3b>(i,j)[2] = 255;
            } else {
                binarized.at<Vec3b>(i,j)[0] = 0;
                binarized.at<Vec3b>(i,j)[1] = 0;
                binarized.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
    return binarized;
}

// Determine the pixels in orig where one of the values of all three
// channels are greater than the threshold
Mat PAOLProcUtils::binarizeOr(const Mat& orig, int threshold) {
    Mat binarized = Mat::zeros(orig.size(), orig.type());
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            if(orig.at<Vec3b>(i,j)[0] >= threshold ||
                    orig.at<Vec3b>(i,j)[1] >= threshold ||
                    orig.at<Vec3b>(i,j)[2] >= threshold) {
                binarized.at<Vec3b>(i,j)[0] = 255;
                binarized.at<Vec3b>(i,j)[1] = 255;
                binarized.at<Vec3b>(i,j)[2] = 255;
            } else {
                binarized.at<Vec3b>(i,j)[0] = 0;
                binarized.at<Vec3b>(i,j)[1] = 0;
                binarized.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
    return binarized;
}

Mat PAOLProcUtils::binarize(const Mat& orig, int threshold) {
    Mat binarized = Mat::zeros(orig.size(), orig.type());
    for(int i = 0; i < orig.rows; i++) {
        for(int j = 0; j < orig.cols; j++) {
            if(orig.at<Vec3b>(i,j)[0] >= threshold ) {
                binarized.at<Vec3b>(i,j)[0] = 255;
                binarized.at<Vec3b>(i,j)[1] = 255;
                binarized.at<Vec3b>(i,j)[2] = 255;
            } else {
                binarized.at<Vec3b>(i,j)[0] = 0;
                binarized.at<Vec3b>(i,j)[1] = 0;
                binarized.at<Vec3b>(i,j)[2] = 0;
            }
        }
    }
    return binarized;
}

// Determine the pixels in orig where the value of the blue channel
// is greater than the threshold. Ignores pixels on the image border
// Arguments:
//    orig: The image to threshold
//    threshold: The minimum value of the blue channel
//    size: The width of the image border to ignore
Mat PAOLProcUtils::thresholdOnBlueChannel(const Mat& orig, int blueThresh, int size) {
    Mat ret = Mat::zeros(orig.size(), orig.type());
    //for every pixel
    for(int y = size; y < orig.rows - size; y++) {
        for(int x = size ; x < orig.cols - size; x++) {
            //if the blue channel in orig is greater than the theshold
            if(orig.at<Vec3b>(y,x)[0] > blueThresh) {
                //brighten edge
                ret.at<Vec3b>(y,x)[0]=255;
                ret.at<Vec3b>(y,x)[1]=255;
                ret.at<Vec3b>(y,x)[2]=255;
            }
        }
    }
    return ret;
}

// Run a specialized edge filter on the original image. The derivative in the horizontal
// direction is stored in the red channel (for vertical edges), the derivative in the
// vertical direction is stored in the green channel (for horizontal edges), and the
// sum is stored in the blue channel
Mat PAOLProcUtils::pDrift(const Mat& orig) {
    int temp,total;
    Mat ret = Mat::zeros(orig.size(), orig.type());

    //for every pixel in image (excludeing edges where perocess would break
    for(int y = 0; y < orig.rows -1; y++)
        for(int x = 0; x < orig.cols -1; x++)
        {
            //look for edges in the vertical direction using a variation on a Sobel filter
            //[1 -1]
            temp = (
                        //y,x+1
                        abs(orig.at<Vec3b>(y,x)[0] - orig.at<Vec3b>(y,x+1)[0])+
                    abs(orig.at<Vec3b>(y,x)[1] - orig.at<Vec3b>(y,x+1)[1])+
                    abs(orig.at<Vec3b>(y,x)[2] - orig.at<Vec3b>(y,x+1)[2])
                    );

            if(temp > 255)
                temp = 255;
            //write the vertical edge information to the red color channel
            ret.at<Vec3b>(y,x)[2] = temp;
            total = temp;

            //run the same filters in the vertical direction to look for edges in the
            //horizontal direction.
            temp = (
                        //y+1,x
                        abs(orig.at<Vec3b>(y,x)[0] - orig.at<Vec3b>(y+1,x)[0])+
                    abs(orig.at<Vec3b>(y,x)[1] - orig.at<Vec3b>(y+1,x)[1])+
                    abs(orig.at<Vec3b>(y,x)[2] - orig.at<Vec3b>(y+1,x)[2])
                    );
            if(temp > 255)
                temp = 255;
            total+=temp;
            if(total > 255)
                total = 255;

            //write the horizontal edge information to the green color channel
            ret.at<Vec3b>(y,x)[1] = temp;
            //write the addition of the horizontal and vertical edges found to the blue color channel
            ret.at<Vec3b>(y,x)[0] = total;
        }
    return ret;
}

// Given approximate borders (enclosures) of the marker strokes, fill in the enclosed regions
Mat PAOLProcUtils::fillMarkerBorders(const Mat& markerBorders) {
    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                         Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                         Point( dilation_size, dilation_size ) );
    Mat filledMarker;
    dilate(markerBorders, filledMarker, element);
    erode(filledMarker, filledMarker, element);
    element.release();
    return filledMarker;
}

// Locates marker strokes by approximating the borders for the marker strokes, then
// filling in the enclosed regions. This method is faster than the connected
// components method, but not as accurate.
Mat PAOLProcUtils::findMarkerWithMarkerBorders(const Mat &whiteboardImage) {
    Mat temp = boxBlur(whiteboardImage, 1);
    temp = pDrift(temp);
    temp = thresholdOnBlueChannel(temp, 15, 3);
    //temp = fillMarkerBorders(temp);
    return temp;
}

// Runs the Difference of Gaussians (DoG) edge detector on the given image.
// TODO: Implement the DoG detector by first subtracting the Gaussian kernels,
//       then filter the result with orig. I can't figure out how to do this.
// Arguments:
//    orig: The image to locate the edges of
//    kerSize: The kernel size for the edge detector. It must be odd, otherwise
//             the filter fails.
//    sig1: The sigma of the first Gaussian blur. Should be larger than sig2.
//    sig2: The sigma of the second Gaussian blur.
Mat PAOLProcUtils::getDoGEdges(const Mat& orig, int kerSize, float sig1, float sig2) {
    Mat g1, g2;
    GaussianBlur(orig, g1, Size(kerSize, kerSize), sig1);
    GaussianBlur(orig, g2, Size(kerSize, kerSize), sig2);
    return g1-g2;
}

// Change the brightness levels of the given image.
// Arguments:
//    orig: The image to adjust the brightness levels of
//    lo: The low threshold of the input brightnesses
//    hi: The high threshold of the input brightnesses
//    gamma: The curvature of the level remapping
Mat PAOLProcUtils::adjustLevels(const Mat& orig, int lo, int hi, double gamma) {
    return 255/pow(hi-lo, 1/gamma)*(orig-lo)^(1/gamma);
}

// Given a binary image, locate the connected components in the image
// WARNING: The returned array must be manually destroyed after use.
int** PAOLProcUtils::getConnectedComponents(const Mat& components) {
    int** componentLabels = new int*[components.rows];
    for(int i = 0; i < components.rows; i++) {
        componentLabels[i] = new int[components.cols];
    }

    // The disjoint set structure that keeps track of component classes
    UF compClasses(DEFAULT_NUM_CC);
    // Counter for the components in the image
    int regCounter = 1;
    for(int i = 0; i < components.rows; i++) {
        for(int j = 0; j < components.cols; j++) {
            // Set component class if mask is white at current pixel
            if(components.at<Vec3b>(i, j)[0] == 255) {
                // Check surrounding pixels
                if(i-1 < 0) {
                    // On top boundary, so just check left
                    if(j-1 < 0) {
                        // This is the TL pixel, so set as new class
                        componentLabels[i][j] = regCounter;
                        regCounter++;
                    }
                    else if(componentLabels[i][j-1] == -1) {
                        // No left neighbor, so set pixel as new class
                        componentLabels[i][j] = regCounter;
                        regCounter++;
                    }
                    else {
                        // Assign pixel class to the same as left neighbor
                        componentLabels[i][j] = componentLabels[i][j-1];
                    }
                }
                else {
                    if(j-1 < 0) {
                        // On left boundary, so just check top
                        if(componentLabels[i-1][j] == -1) {
                            // No top neighbor, so set pixel as new class
                            componentLabels[i][j] = regCounter;
                            regCounter++;
                        }
                        else {
                            // Assign pixel class to same as top neighbor
                            componentLabels[i][j] = componentLabels[i-1][j];
                        }
                    }
                    else {
                        // Normal case (get top and left neighbor and reassign classes if necessary)
                        int topClass = componentLabels[i-1][j];
                        int leftClass = componentLabels[i][j-1];
                        if(topClass == -1 && leftClass == -1) {
                            // No neighbor exists, so set pixel as new class
                            componentLabels[i][j] = regCounter;
                            regCounter++;
                        }
                        else if(topClass == -1 && leftClass != -1) {
                            // Only left neighbor exists, so copy its class
                            componentLabels[i][j] = leftClass;
                        }
                        else if(topClass != -1 && leftClass == -1) {
                            // Only top neighbor exists, so copy its class
                            componentLabels[i][j] = topClass;
                        }
                        else {
                            // Both neighbors exist
                            int minNeighbor = std::min(componentLabels[i-1][j], componentLabels[i][j-1]);
                            int maxNeighbor = std::max(componentLabels[i-1][j], componentLabels[i][j-1]);
                            componentLabels[i][j] = minNeighbor;
                            // If we have differing neighbor values, merge them
                            if(minNeighbor != maxNeighbor) {
                                compClasses.merge(minNeighbor, maxNeighbor);
                            }
                        }
                    }
                }
            }
            else {
                // The pixel is black, so do not give a component label
                componentLabels[i][j] = -1;
            }
        }
    }
    // Unify the labels such that every pixel in a component has the same label
    for(int i=0; i < components.rows; i++) {
        for(int j=0; j < components.cols; j++) {
            componentLabels[i][j] = compClasses.find(componentLabels[i][j]);
        }
    }
    return componentLabels;
}

// Given the connected components and an additional binary image, keep the components
// that overlap with a pixel from the other binary image.
// Arguments:
//    compsImg: The connected components (labels not yet assigned)
//    keepCompLocs: The pixels that a component must overlap with to be kept
Mat PAOLProcUtils::filterConnectedComponents(const Mat& compsImg, const Mat& keepCompLocs) {
    // Get the component labels
    int** components = getConnectedComponents(compsImg);

    // Initialize lookup table of components to keep (initially keep no components)
    vector<bool> componentsToKeep(DEFAULT_NUM_CC, false);
    for(int i=0; i < keepCompLocs.rows; i++) {
        for(int j=0; j < keepCompLocs.cols; j++) {
            // If there is a component at the pixel and the pixel from keepCompLocs is
            // white, then keep the component
            if(keepCompLocs.at<Vec3b>(i,j)[2] == 255 && components[i][j] > 0) {
                unsigned int compLabel = components[i][j];
                // Resize the lookup table if it cannot contain the found comp label
                if(compLabel >= componentsToKeep.size())
                    componentsToKeep.resize(2*compLabel, false);
                componentsToKeep[compLabel] = true;
            }
        }
    }

    // Add the components that intersected with the edge detector
    Mat filteredComps = Mat::zeros(compsImg.size(), compsImg.type());
    for(int i=0; i < keepCompLocs.rows; i++) {
        for(int j=0; j < keepCompLocs.cols; j++) {
            // If the pixel is part of a kept component, add it to the output
            int compLabel = components[i][j];
            if(compLabel > 0 && componentsToKeep[compLabel]) {
                filteredComps.at<Vec3b>(i,j)[0] = 255;
                filteredComps.at<Vec3b>(i,j)[1] = 255;
                filteredComps.at<Vec3b>(i,j)[2] = 255;
            }
        }
    }

    // Free memory used by components array
    for(int i = 0; i < compsImg.rows; i++) {
        delete [] components[i];
    }
    delete [] components;

    return filteredComps;
}

// Locate the marker strokes with the DoG and connected components approach.
// First, find the DoG edges and threshold them. Then, use pDrift to determine
// which connected components from the threholded DoG edges should be kept.
Mat PAOLProcUtils::findMarkerWithCC(const Mat& orig) {
    Mat markerCandidates = findMarkerStrokeCandidates(orig);
    Mat markerLocations = findMarkerStrokeLocations(orig);
    return filterConnectedComponents(markerCandidates, markerLocations);
}

Mat PAOLProcUtils::findMarkerStrokeCandidates(const Mat& orig) {
    Mat markerCandidates = getDoGEdges(orig, 13, 17, 1);
    markerCandidates = adjustLevels(markerCandidates, 0, 4, 1);
    return binarizeAnd(markerCandidates, 10);
}

Mat PAOLProcUtils::findMarkerStrokeLocations(const Mat& orig) {
    Mat markerLocations = pDrift(orig);
    return binarize(markerLocations, 12);
}

///////////////////////////////////////////////////////////////////
///
///   Methods to enhance the whiteboard
///
///////////////////////////////////////////////////////////////////

// Do a box blurring on the original image
// Arguments:
//    orig: The image to blur
//    size: The radius of the box blur kernel
Mat PAOLProcUtils::boxBlur(const Mat& orig, int size) {
    Mat ret;
    cv::blur(orig, ret, Size(2*size+1, 2*size+1));
    return ret;
}

// Approximate the whiteboard color at each pixel by splitting the image
// into cells, then finding the average value of the brightest 25% of
// pixels in the cell.
// Arguments:
//    whiteboardImg: The unmodified whiteboard image to extract the whiteboard colors of
//    size: The size of the cells
Mat PAOLProcUtils::getAvgWhiteboardColor(const Mat& whiteboardImg, int size) {
    Mat ret = Mat::zeros(whiteboardImg.size(), whiteboardImg.type());
    int x,y,xx,yy;
    int count,color,thresh;
    vector <int> pix;
    vector <int> ave;

    //go through the image by squares of radius size
    for (x=0;x<whiteboardImg.cols;x+=size)
        for (y=0;y<whiteboardImg.rows;y+=size){
            pix.clear();
            ave.clear();

            //within each square create a vector pix that hold all brightness values
            //for the pixels
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    color/=3;
                    pix.push_back(color);
                }

            //clear the average pixel values
            for (int c=0;c<3;c++)
                ave.push_back(0);

            //sort the vector of brightness pixels (low to high)
            sort(pix.begin(),pix.end());
            //figure out what the brightness theshold is for the brightest 25%
            thresh=pix[pix.size()*3/4];
            count=0;

            //for all the pixels in the square add the color components to the averge vector
            //if the brightness is over the theshold
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    color/=3;

                    if(color>=thresh){
                        count++;
                        for (int c=0;c<3;c++)
                            ave[c]+=whiteboardImg.at<Vec3b>(yy,xx)[c];
                    }
                }
            //figure out the average brightness of each channel for the brightest pixels
            for (int c=0;c<3;c++)
                ave[c]/=count;

            //set the pixels in the mask to the average brightness of the image, square by square
            for(xx=x; xx<x+size && xx<whiteboardImg.cols; xx++)
                for (yy=y; yy<y+size && yy<whiteboardImg.rows; yy++){
                    for (int c=0;c<3;c++)
                        ret.at<Vec3b>(yy,xx)[c]=ave[c];
                }
        }
    return ret;
}

// Given a whiteboard image, darken the text by scaling the difference from the average whiteboard color
Mat PAOLProcUtils::raiseMarkerContrast(const Mat& whiteboardImg){
    Mat hiContrastImg = Mat::zeros(whiteboardImg.size(), whiteboardImg.type());
    Mat avg = getAvgWhiteboardColor(whiteboardImg, 10);

    //for every pixel in the image and for every color channel
    for(int x = 0; x < whiteboardImg.cols; x++)
        for(int y = 0; y < whiteboardImg.rows; y++){
            for(int c=0;c<3;c++){
                int dif;
                //if the pixel is not 0 (just put in so that we don't divide by 0)
                if (whiteboardImg.at<Vec3b>(y,x)[c]>0){
                    //take the brightness of the pixel and divide it by what white is in
                    //that location (average from mask)
                    if(avg.at<Vec3b>(y,x)[c]!=0){
                        dif=255*whiteboardImg.at<Vec3b>(y,x)[c]/avg.at<Vec3b>(y,x)[c];
                    }else{
                        dif=255;
                    }
                    //if it's brighter then white turn it white
                    if (dif>255)
                        dif=255;
                } else {
                    //if the average pixel color is 0 turn it white
                    dif=255;
                }
                //double the distance each color is from white to make text darker
                dif=255-(255-dif)*2;
                if (dif<0)
                    dif=0;
                hiContrastImg.at<Vec3b>(y,x)[c]=dif;
            }
        }
    avg.release();
    return hiContrastImg;
}

// Given a whiteboard image and the marker locations, make the non-marker pixels white
// Arguments:
//    whiteboardImg: The whiteboard image to modify
//    markerPixels: A binary image where the marker pixel locations are white
Mat PAOLProcUtils::whitenWhiteboard(const Mat& whiteboardImg, const Mat& markerPixels) {
    Mat ret = whiteboardImg.clone();
    //for every pixel
    for(int y = 0; y < whiteboardImg.rows; y++)
        for(int x = 0; x < whiteboardImg.cols; x++){
            //if there isn't and edge (text) in that location turn the pixel white
            if (markerPixels.at<Vec3b>(y,x)[1]<50){
                ret.at<Vec3b>(y,x)[0]=255;
                ret.at<Vec3b>(y,x)[1]=255;
                ret.at<Vec3b>(y,x)[2]=255;
            }
        }
    return ret;
}

Mat PAOLProcUtils::rectifyImage(const Mat& whiteboardImg, const WBCorners& corners){
    // Set where the whiteboard corners are in the image
    vector<Point2f> cornersInImage;
    cornersInImage.push_back(corners.TL);
    cornersInImage.push_back(corners.TR);
    cornersInImage.push_back(corners.BR);
    cornersInImage.push_back(corners.BL);

    double dWidthT=sqrt(pow(corners.TL.x-corners.TR.x,2)+pow(corners.TL.y-corners.TR.y,2));
    int iWidthT=(int)dWidthT;
    double dHeightL=sqrt(pow(corners.TL.x-corners.BL.x,2)+pow(corners.TL.y-corners.BL.y,2));
    int iHeightL=(int)dHeightL;
    double dWidthB=sqrt(pow(corners.BL.x-corners.BR.x,2)+pow(corners.BL.y-corners.BR.y,2));
    int iWidthB=(int)dWidthB;
    double dHeightR=sqrt(pow(corners.TR.x-corners.BR.x,2)+pow(corners.TR.y-corners.BR.y,2));
    int iHeightR=(int)dHeightR;
    int width=(iWidthB>iWidthT)? iWidthB : iWidthT;
    int height=(iHeightL>iHeightR)? iHeightL : iHeightR;

    // Set where the whiteboard corners should end up in the image (ie. the corners of the whole image)
    vector<Point2f> finalCorners;
    finalCorners.push_back(Point2f(0, 0));
    finalCorners.push_back(Point2f(width, 0));
    finalCorners.push_back(Point2f(width, height));
    finalCorners.push_back(Point2f(0, height));

    // Get the transform matrix
    Mat transform = getPerspectiveTransform(cornersInImage, finalCorners);
    // Do the perspective correction
    Mat ret;
    warpPerspective(whiteboardImg, ret, transform, Size(width,height));
    transform.release();
    return ret;
}

// Given a whiteboard image, draw red lines through the long straight lines of
// the image.
Mat PAOLProcUtils::findWhiteboardBorders(Mat& whiteboardImg) {
    // Find edges with Canny detector
    Mat cannyEdges;
    Canny(whiteboardImg, cannyEdges, 50, 200, 3);

    // detect lines
    vector<Vec2f> lines;
    HoughLines(cannyEdges, lines, 1, CV_PI/180, 250, 0, 0 );

    // draw lines
    Mat ret = whiteboardImg.clone();
    for( size_t i = 0; i < lines.size(); i++ )
    {
        float rho = lines[i][0], theta = lines[i][1];
        Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a*rho, y0 = b*rho;
        pt1.x = cvRound(x0 + 2000*(-b));
        pt1.y = cvRound(y0 + 2000*(a));
        pt2.x = cvRound(x0 - 2000*(-b));
        pt2.y = cvRound(y0 - 2000*(a));
        line( ret, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
    cannyEdges.release();
    return ret;
}

// Make marker-whiteboard transition smoother
// Arguments:
//    whiteWhiteboardImage: The whiteboard image where the non-marker pixels are white
//                          (see whitenWhiteboard)
Mat PAOLProcUtils::smoothMarkerTransition(const Mat& whiteWhiteboardImage) {
    Mat blurred;
    int temp;
    GaussianBlur(whiteWhiteboardImage, blurred, Size(5,5), .8);
    // Overlay blurred version with pixels from whiteWhiteboardImage
    for(int i = 0; i < whiteWhiteboardImage.rows; i++) {
        for(int j = 0; j < whiteWhiteboardImage.cols; j++) {
            // If whiteWhiteboardImage is not white at this pixel, it is marker, so fill in marker color
            if(whiteWhiteboardImage.at<Vec3b>(i,j)[0] != 255
                    || whiteWhiteboardImage.at<Vec3b>(i,j)[1] != 255
                    || whiteWhiteboardImage.at<Vec3b>(i,j)[2] != 255) {
                //try to make the text not crap
                temp=255-(whiteWhiteboardImage.at<Vec3b>(i,j)[0]-255)*2;
                if(temp<0)
                    temp=0;
                blurred.at<Vec3b>(i, j)[0] = temp;
                temp=255-(whiteWhiteboardImage.at<Vec3b>(i,j)[1]-255)*2;
                if(temp<0)
                    temp=0;
                blurred.at<Vec3b>(i, j)[1] = temp;
                temp=255-(whiteWhiteboardImage.at<Vec3b>(i,j)[2]-255)*2;
                if(temp<0)
                    temp=0;
                blurred.at<Vec3b>(i, j)[2] = temp;
            }
        }
    }
    return blurred;
}

int PAOLProcUtils::countNoneWhite(const Mat &background){
    int count=0;

    for(int i = 0; i < background.rows; i++) {
        for(int j = 0; j < background.cols; j++) {
            // If whiteWhiteboardImage is not white at this pixel, it is marker, so fill in marker color
            if(background.at<Vec3b>(i,j)[0] != 255
                    || background.at<Vec3b>(i,j)[1] != 255
                    || background.at<Vec3b>(i,j)[2] != 255) {
                count++;
            }
        }
    }
    return count;
}
Mat PAOLProcUtils::getNotWhite(const Mat &whiteboard){
    Mat out = Mat::zeros(whiteboard.size(), whiteboard.type());

    for(int i = 0; i < whiteboard.rows; i++) {
        for(int j = 0; j < whiteboard.cols; j++) {
            // If whiteWhiteboardImage is not white at this pixel, it is marker, so fill in marker color
            if(whiteboard.at<Vec3b>(i,j)[0] != 255
                    || whiteboard.at<Vec3b>(i,j)[1] != 255
                    || whiteboard.at<Vec3b>(i,j)[2] != 255) {
                out.at<Vec3b>(i,j)[0]=255;
                out.at<Vec3b>(i,j)[1]=255;
                out.at<Vec3b>(i,j)[2]=255;
            }
        }
    }
    return out;
}

///////////////////////////////////////////////////////////////////
///
///   Update a model (marker or whiteboard model)
///
///////////////////////////////////////////////////////////////////

// Arguments:
//    oldModel: The previous version of the model
//    newInfo: The image containing the information to update the model with
//    oldInfoMask: A binary image indicating (with white pixels) where the model should NOT be updated
Mat PAOLProcUtils::updateModel(const Mat& oldModel, const Mat& newInfo, const Mat& oldInfoMask) {
    // Throw exception if one of the given arguments has no image data
    // Mostly useful to protect against updating a nonexistent model
    if(!oldModel.data || !newInfo.data || !oldInfoMask.data) {
        throw std::invalid_argument("updateBack3: Attempted to update a model with missing data.");
    }

    Mat updatedModel = oldModel.clone();

    //for every pixel in the image
    for (int y = 0; y < updatedModel.rows; y++) {
        for (int x = 0; x < updatedModel.cols; x++) {
            //if there was no movement at that pixel
            if (oldInfoMask.at<Vec3b>(y,x)[0] == 0) {
                //update the whiteboard model at that pixel
                for (int c=0;c<3;c++){
                    updatedModel.at<Vec3b>(y,x)[c]=newInfo.at<Vec3b>(y,x)[c];
                }
            }
        }
    }

    return updatedModel;
}

// Find the percentage of pixels that differ between the old and new marker models
// Arguments:
//    oldMarkerModel: A binary image indicating (in white) where the marker is in the old model
//    newMarkerModel: A binary image indicating (in white) where the marker is in the new model
float PAOLProcUtils::findMarkerModelDiffs(const Mat& oldMarkerModel, const Mat& newMarkerModel) {
    float count = 0;
    for(int i = 0; i < oldMarkerModel.rows; i++) {
        for(int j = 0; j < oldMarkerModel.cols; j++) {
            if(oldMarkerModel.at<Vec3b>(i,j)[0] != newMarkerModel.at<Vec3b>(i,j)[0])
                count++;
        }
    }
    return count / (oldMarkerModel.rows*oldMarkerModel.cols);
}

float PAOLProcUtils::findMarkerStrokeDiffs(const Mat& oldMarkerModel, const Mat& newMarkerModel) {
    Mat oldModel=growGreen(oldMarkerModel,6);
    Mat newModel=growGreen(newMarkerModel,6);
    float count = 0;
    for(int i = 0; i < oldModel.rows; i++) {
        for(int j = 0; j < oldModel.cols; j++) {
            if(oldModel.at<Vec3b>(i,j)[1] != newModel.at<Vec3b>(i,j)[1])
                if(oldModel.at<Vec3b>(i,j)[1] != newModel.at<Vec3b>(i,j)[0] &&
                        oldModel.at<Vec3b>(i,j)[0] != newModel.at<Vec3b>(i,j)[1])
                    count++;
        }
    }
    newModel.release();
    return count / (oldModel.rows*oldModel.cols);
}

Mat PAOLProcUtils::findMarkerStrokeDiffs2(const Mat& oldMarkerModel, const Mat& newMarkerModel) {
    Mat oldModel=growGreen(oldMarkerModel,6);
    Mat newModel=growGreen(newMarkerModel,6);
    Mat ret=oldMarkerModel.clone();
    float count = 0;

    for(int i = 0; i < oldModel.rows; i++) {
        for(int j = 0; j < oldModel.cols; j++) {
            ret.at<Vec3b>(i,j)[0]=0;
            ret.at<Vec3b>(i,j)[1]=0;
            ret.at<Vec3b>(i,j)[2]=0;
            if(oldModel.at<Vec3b>(i,j)[1] != newModel.at<Vec3b>(i,j)[1]){
                ret.at<Vec3b>(i,j)[1]=255;
                if(oldModel.at<Vec3b>(i,j)[1] != newModel.at<Vec3b>(i,j)[0] &&
                        oldModel.at<Vec3b>(i,j)[0] != newModel.at<Vec3b>(i,j)[1]){
                    count++;
                    ret.at<Vec3b>(i,j)[2]=255;
                    ret.at<Vec3b>(i,j)[1]=0;
                }
            } else if(oldModel.at<Vec3b>(i,j)[0] != newModel.at<Vec3b>(i,j)[0]){
                ret.at<Vec3b>(i,j)[0]=255;
            }
        }
    }
    oldModel.release();
    newModel.release();
    return ret;
}

//function to make whiteboard readable
Mat PAOLProcUtils::darkenText( Mat &pdrift, const Mat &orig){
    int temp;
    Mat tempOut;
    Mat ret=orig.clone();

    //for every pixel
    for(int y = 0; y < orig.rows; y++)
        for(int x = 0; x < orig.cols; x++){
            //write edge information from blue into green channel and zero out red
            pdrift.at<Vec3b>(y,x)[1]=pdrift.at<Vec3b>(y,x)[0];
            if (pdrift.at<Vec3b>(y,x)[1]>20)
                pdrift.at<Vec3b>(y,x)[1]=255;
            pdrift.at<Vec3b>(y,x)[2]=0;
        }

    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                           Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                           Point( dilation_size, dilation_size ) );

    dilate(pdrift, tempOut, element);
    erode(tempOut, tempOut, element);

    //for every pixel
    for(int y = 0; y < orig.rows; y++)
        for(int x = 0; x < orig.cols; x++){
            //code to make it look pretty on the pdrift
            if (pdrift.at<Vec3b>(y,x)[1]!=255 && tempOut.at<Vec3b>(y,x)[1]>50){
                pdrift.at<Vec3b>(y,x)[2]=255;
            }

            //if there isn't and edge (text) in that location turn the pixel white
            if (tempOut.at<Vec3b>(y,x)[1]<50){
                ret.at<Vec3b>(y,x)[0]=215;
                ret.at<Vec3b>(y,x)[1]=215;
                ret.at<Vec3b>(y,x)[2]=215;
            } else {
                temp=ret.at<Vec3b>(y,x)[0];
                temp-=40;
                if(temp<0)
                    temp=0;
                ret.at<Vec3b>(y,x)[0]=temp;

                temp=ret.at<Vec3b>(y,x)[1];
                temp-=40;
                if(temp<0)
                    temp=0;
                ret.at<Vec3b>(y,x)[1]=temp;

                temp=ret.at<Vec3b>(y,x)[2];
                temp-=40;
                if(temp<0)
                    temp=0;
                ret.at<Vec3b>(y,x)[2]=temp;
            }
        }
    tempOut.release();
    element.release();
    return ret;
}

Mat PAOLProcUtils::refineImage(const Mat &current, const Mat &avg){
    Mat hiContrastImg = Mat::zeros(current.size(), current.type());
    int aveBright,curBright,dif;
    double alpha,beta,c1,c2,gu,pixOut,colorDouble;
    alpha=3;


    //for every pixel in the image and for every color channel
    for(int x = 0; x < current.cols; x++)
        for(int y = 0; y < current.rows; y++){
            aveBright=0;
            curBright=0;
            for(int c=0;c<3;c++){
                aveBright+=avg.at<Vec3b>(y,x)[c];
                curBright+=current.at<Vec3b>(y,x)[c];
            }
            aveBright/=3;
            curBright/=3;
            if(aveBright<=curBright+3){
                for(int c=0;c<3;c++){
                    hiContrastImg.at<Vec3b>(y,x)[c]=255;
                }
            } else {
                curBright=0;
                for(int c=0;c<3;c++){
                    if (avg.at<Vec3b>(y,x)[c]>0){
                        //take the brightness of the pixel and divide it by what white is in
                        //that location (average from mask)
                        curBright+=255*current.at<Vec3b>(y,x)[c]/avg.at<Vec3b>(y,x)[c];
                    }
                }
                curBright/=3;

                for(int c=0;c<3;c++){
                    if (avg.at<Vec3b>(y,x)[c]>0){
                        //take the brightness of the pixel and divide it by what white is in
                        //that location (average from mask)
                        dif=255*current.at<Vec3b>(y,x)[c]/avg.at<Vec3b>(y,x)[c];

                        //if it's brighter then white turn it white
                        if (dif>255)
                            dif=255;
                    } else {
                        //if the average pixel color is 0 set it to 0
                        dif=0;
                    }

                    beta=alpha*2*(curBright);
                    colorDouble=((double)dif)/255.0;
                    gu=1/(1+exp(-alpha*colorDouble+beta/255.0));
                    c1=1/(1+exp(beta/255.0));
                    c2=1/(1+exp(-alpha+beta/255.0))-c1;
                    pixOut=(gu-c1)/c2;
                    pixOut*=255;
                    hiContrastImg.at<Vec3b>(y,x)[c]=(int)pixOut;
                }
            }
        }

    return hiContrastImg;
}

Mat PAOLProcUtils::CLAHE(const Mat &orig){
    Mat lab_image;
    cvtColor(orig, lab_image, CV_BGR2Lab);

    // Extract the L channel
    vector<cv::Mat> lab_planes(3);
    split(lab_image, lab_planes);  // now we have the L image in lab_planes[0]

    // apply the CLAHE algorithm to the L channel
    Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(4);
    Mat dst;
    clahe->apply(lab_planes[0], dst);

    // Merge the the color planes back into an Lab image
    dst.copyTo(lab_planes[0]);
    merge(lab_planes, lab_image);

    // convert back to RGB
    Mat image_clahe;
    cvtColor(lab_image, image_clahe, CV_Lab2BGR);

    lab_image.release();
    dst.release();
    return image_clahe;
}

Mat PAOLProcUtils::enhanceColor(const Mat &orig){
    Mat ave,out;
    int dif;

    blur(orig,ave,Size(20,20));

    for(int x = 0; x < orig.cols; x++)
        for(int y = 0; y < orig.rows; y++){
            for(int c=0;c<3;c++){
                if(ave.at<Vec3b>(y,x)[c]>0){
                    dif=255*orig.at<Vec3b>(y,x)[c]/ave.at<Vec3b>(y,x)[c];
                    //if it's brighter then white turn it white
                    if (dif>255)
                        dif=255;
                } else {
                    //if the average pixel color is 0 turn it white
                    dif=255;
                }
                dif=255-(255-dif)*2;
                if (dif<0)
                    dif=0;
                ave.at<Vec3b>(y,x)[c]=dif;
            }
        }
    GaussianBlur(ave, out, cv::Size(3, 3), 3);
    addWeighted(ave, 1.5, out, -0.5, 0, out);

    ave.release();
    return out;
}

// Find the percentage of pixels that differ between the old and new computer images
float PAOLProcUtils::getVGADifferences(const Mat& oldFrame, const Mat& newFrame)
{
    // If the image sizes do not match, we consider it a 100% difference
    if(oldFrame.rows != newFrame.rows || oldFrame.cols != newFrame.cols)
        return 1;

    // Process the difference between two images with the same resolution
    bool diff;
    float numDiff;
    int dist;
    bool first;
    int cenx;
    int ceny;
    int total;
    //mask is set to a blank state
    Mat mask = Mat::zeros(oldFrame.size(), oldFrame.type());

    numDiff = 0;
    first = true;
    //distance --
    dist = 0;
    //for every row
    for (int y = WINDOW_SIZE; y < (oldFrame.rows-(WINDOW_SIZE+1+BOTTOM_MASK)); y++)
    {
        //for every column
        for (int x = WINDOW_SIZE; x < (oldFrame.cols-(WINDOW_SIZE+1)); x++)
        {
            diff = false;
            //for each color channel
            for(int i = 0; i < 3; i++)
            {
                //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
                if(abs((double)newFrame.at<Vec3b>(y,x)[i]-(double)oldFrame.at<Vec3b>(y,x)[i])>PIXEL_DIFF_THRESHOLD)
                    diff = true;
            }
            if(diff)
            {
                //std::cout<<"First if dif size: "<<size<<std::endl;
                //mask.at<Vec3b>(y,x)[1]=255;
                // for all the pixels surrounding the current pixel
                for(int yy = y-WINDOW_SIZE; yy < y+WINDOW_SIZE; yy++)
                {
                    for(int xx = x-WINDOW_SIZE; xx < x+WINDOW_SIZE; xx++)
                    {
                        //for each color channel
                        for(int ii = 0; ii < 3; ii++)
                        {
                            //ignore all differneces found at the edges; sometimes pixels get lost in tranmission
                            if(abs(((double)(newFrame.at<Vec3b>(yy,xx)[ii]))-(((double)(newFrame.at<Vec3b>((yy+1),xx)[ii])))>BORDER_DIFF_THRESHOLD))
                                diff = false;
                            if(abs(((double)(newFrame.at<Vec3b>(yy,xx)[ii]))-(((double)(newFrame.at<Vec3b>(yy,(xx+1))[ii])))>BORDER_DIFF_THRESHOLD))
                                diff = false;
                        }
                    }
                }
            }
            if(diff)
            {
                //std::cout<<"Second if diff"<<std::endl;
                numDiff++;
                //calculates total difference and modifies the mask accordingly
                total = abs((double)newFrame.at<Vec3b>(y,x)[0]-(double)oldFrame.at<Vec3b>(y,x)[0]) +
                        abs((double)newFrame.at<Vec3b>(y,x)[1]-(double)oldFrame.at<Vec3b>(y,x)[1]) +
                        abs((double)newFrame.at<Vec3b>(y,x)[2]-(double)oldFrame.at<Vec3b>(y,x)[2]);
                if(total > 512)
                {
                    mask.at<Vec3b>(y,x)[0] = 255;
                }
                if(total > 255)
                {
                    mask.at<Vec3b>(y,x)[1] = 255;
                    numDiff++;
                }
                mask.at<Vec3b>(y,x)[2]=255;
                //sets location of first differnce found
                if(first)
                {
                    first = false;
                    cenx = x;
                    ceny = y;
                }
                //std::cout<<"Difference x: "<<x<<" cenx: "<<cenx<<" y:"<<y<<" ceny: "<<ceny<<std::endl;
                //distance between pixels
                dist+=sqrt(((x-cenx)*(x-cenx))+((y-ceny)*(y-ceny)));
            }
        }
    }

    mask.release();
    //std::cout<<"Difference dist: "<<dist<<std::endl;
    if((dist<10000))//&&(maskBottom>0))
        return 0;
    else
        return numDiff / (oldFrame.rows*oldFrame.cols);
}
Mat PAOLProcUtils::getWhiteboardDifferences(const Mat &oldFrame, const Mat &newFrame)
{
    Mat out=newFrame.clone();
    // If the image sizes do not match, we consider it a 100% difference
    if(oldFrame.rows != newFrame.rows || oldFrame.cols != newFrame.cols)
        return out;

    // Process the difference between two images with the same resolution
    bool diff;
    //bool first;
    //int dist;
    float numDiff;
    bool oldWhite,newWhite;
    //mask is set to a blank state
    Mat mask = Mat::zeros(oldFrame.size(), oldFrame.type());

    numDiff = 0;
    //first = true;
    //distance --
    //dist = 0;
    //for every row
    for (int y = 0; y < (oldFrame.rows); y++)
    {
        //for every column
        for (int x = 0; x < (oldFrame.cols); x++)
        {
            diff = false;
            //for each color channel
            for(int i = 0; i < 3; i++)
            {
                //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
                if(abs((double)newFrame.at<Vec3b>(y,x)[i]-(double)oldFrame.at<Vec3b>(y,x)[i])>PIXEL_DIFF_THRESHOLD)
                    diff = true;
            }

            if(diff)
            {
                if(oldFrame.at<Vec3b>(y,x)[0]==255 &&
                        oldFrame.at<Vec3b>(y,x)[1]==255 &&
                        oldFrame.at<Vec3b>(y,x)[2]==255){
                    oldWhite=true;
                    mask.at<Vec3b>(y,x)[2] = 255;
                }
                else
                    oldWhite=false;
                if(newFrame.at<Vec3b>(y,x)[0]==255 &&
                        newFrame.at<Vec3b>(y,x)[1]==255 &&
                        newFrame.at<Vec3b>(y,x)[2]==255){
                    newWhite=true;
                    mask.at<Vec3b>(y,x)[1] = 255;
                }else
                    newWhite=false;
                if (newWhite!=oldWhite){
                    mask.at<Vec3b>(y,x)[0] = 255;
                    numDiff++;
                }
                /*
                //std::cout<<"Second if diff"<<std::endl;
                numDiff++;
                //calculates total difference and modifies the mask accordingly
                total = abs((double)newFrame.at<Vec3b>(y,x)[0]-(double)oldFrame.at<Vec3b>(y,x)[0]) +
                        abs((double)newFrame.at<Vec3b>(y,x)[1]-(double)oldFrame.at<Vec3b>(y,x)[1]) +
                        abs((double)newFrame.at<Vec3b>(y,x)[2]-(double)oldFrame.at<Vec3b>(y,x)[2]);
                if(total > 512)
                {
                    mask.at<Vec3b>(y,x)[0] = 255;
                }
                if(total > 255)
                {
                    mask.at<Vec3b>(y,x)[1] = 255;
                    numDiff++;
                }
                mask.at<Vec3b>(y,x)[2]=255;
                //sets location of first differnce found
                if(first)
                {
                    first = false;
                    cenx = x;
                    ceny = y;
                }
                //std::cout<<"Difference x: "<<x<<" cenx: "<<cenx<<" y:"<<y<<" ceny: "<<ceny<<std::endl;
                //distance between pixels
                dist+=sqrt(((x-cenx)*(x-cenx))+((y-ceny)*(y-ceny)));
                */
            }
        }
    }
    out=erodeSize(mask,7);
    numDiff=0;
    for(int x=0;x<out.cols;x++)
        for(int y=0;y<out.rows;y++){
            if(out.at<Vec3b>(y,x)[0]==255){
                numDiff++;
            }
        }
    //qDebug("whitepix=%f",numDiff);
    //std::cout<<"Difference dist: "<<dist<<std::endl;
/*    if((dist<10000))//&&(maskBottom>0))
        return 0;
    else
        return numDiff / (oldFrame.rows*oldFrame.cols);
        */
    mask.release();
    return out;
}

Mat PAOLProcUtils::getErodeDifferencesIm(const Mat &oldFrame, const Mat &newFrame){
    Mat mask = Mat::zeros(oldFrame.size(), oldFrame.type());
    int numDiff = 0;

    //for every row
    for (int y = 0; y < (oldFrame.rows); y++)
    {
        //for every column
        for (int x = 0; x < (oldFrame.cols); x++)
        {
            //for each color channel
            if ((newFrame.at<Vec3b>(y,x)[2]==255 && oldFrame.at<Vec3b>(y,x)[0]!=255) ||
                    (oldFrame.at<Vec3b>(y,x)[2]==255 && newFrame.at<Vec3b>(y,x)[0]!=255)){
                numDiff++;
                for(int i = 0; i < 3; i++)
                {
                    mask.at<Vec3b>(y,x)[i]=255;
                }
            }
        }
    }
    return mask;
}

int PAOLProcUtils::getErodeDifferencesNum(const Mat &oldFrame, const Mat &newFrame){
    int numDiff = 0;

    //for every row
    for (int y = 0; y < (oldFrame.rows); y++)
    {
        //for every column
        for (int x = 0; x < (oldFrame.cols); x++)
        {
            //for each color channel
            if ((newFrame.at<Vec3b>(y,x)[2]==255 && oldFrame.at<Vec3b>(y,x)[0]!=255) ||
                    (oldFrame.at<Vec3b>(y,x)[2]==255 && newFrame.at<Vec3b>(y,x)[0]!=255)){
                numDiff++;
            }
        }
    }
    return numDiff;
}

Mat PAOLProcUtils::erodeSize(const Mat &mask, int size){
    int x,y,xx,yy,count;
    Mat out=Mat::zeros(mask.size(), mask.type());

    for ( y = 1; y < mask.rows-1; y++)
    {
        //for every column
        for ( x = 1; x < mask.cols-1; x++)
        {
            if(mask.at<Vec3b>(y,x)[0]==255){
                count=-1;
                for(xx=x-1;xx<=x+1;xx++)
                    for(yy=y-1;yy<=y+1;yy++){
                        if(mask.at<Vec3b>(yy,xx)[0]==255)
                            count++;
                    }
                if(count>=size)
                    for(int i = 0; i < 3; i++)
                        out.at<Vec3b>(y,x)[i]=255;
            }
        }
    }
    return out;
}

//erode any pixel that is not surrounded by SIZE pixels that are also highlighted
Mat PAOLProcUtils::erodeSizeGreen(const Mat &mask, int size){
    int x,y,xx,yy,count;
    Mat out=mask.clone();

    for ( y = 1; y < mask.rows-1; y++)
    {
        //for every column
        for ( x = 1; x < mask.cols-1; x++)
        {
            out.at<Vec3b>(y,x)[2]=0;
            if(mask.at<Vec3b>(y,x)[1]==255){
                count=-1;
                for(xx=x-1;xx<=x+1;xx++)
                    for(yy=y-1;yy<=y+1;yy++){
                        if(mask.at<Vec3b>(yy,xx)[1]==255)
                            count++;
                    }
                if(count>=size){
                    out.at<Vec3b>(y,x)[2]=255;
                    //out.at<Vec3b>(y,x)[0]=0;
                }
            }
        }
    }
    return out;
}
int PAOLProcUtils::countDifferences(const Mat &difs){
    int numDiff=0;
    for(int x=0;x<difs.cols;x++)
        for(int y=0;y<difs.rows;y++){
            if(difs.at<Vec3b>(y,x)[0]==255){
                numDiff++;
            }
        }
    return numDiff;
}


// Sort the given whiteboard corners, assuming that corners.TL might not be the top-left corner, or corners.BR
// might not be the bottom-left corner, etc. Implementation based on the OpenCV tutorial
// http://opencv-code.com/tutorials/automatic-perspective-correction-for-quadrilateral-objects/
void PAOLProcUtils::sortCorners(WBCorners &corners) {
    // Store the corners as a vector
    vector<Point2f> allCorners;
    allCorners.push_back(corners.TL);
    allCorners.push_back(corners.TR);
    allCorners.push_back(corners.BL);
    allCorners.push_back(corners.BR);

    // Get center of mass of the corners (average of the four coordinates)
    Point2f center = .25 * (corners.TL + corners.TR + corners.BL + corners.BR);

    // Get the two top and two bottom points
    vector<Point2f> topPoints, bottomPoints;
    for(unsigned int i = 0; i < allCorners.size(); i++) {
        // Put the corners above the center in topPoints, put the rest in bottomPoints
        if(allCorners[i].y < center.y)
            topPoints.push_back(allCorners[i]);
        else
            bottomPoints.push_back(allCorners[i]);
    }

    corners.TL = (topPoints[0].x > topPoints[1].x) ? topPoints[1] : topPoints[0];
    corners.TR = (topPoints[0].x > topPoints[1].x) ? topPoints[0] : topPoints[1];
    corners.BL = (bottomPoints[0].x > bottomPoints[1].x) ? bottomPoints[1] : bottomPoints[0];
    corners.BR = (bottomPoints[0].x > bottomPoints[1].x) ? bottomPoints[0] : bottomPoints[1];
}

Mat PAOLProcUtils::surrountDifference(Mat& aveIm){
    Mat change;
    Mat view;
    int dif,maxDif;
    change=Mat::zeros(aveIm.size(), aveIm.type());
    view=Mat::zeros(aveIm.size(), aveIm.type());
    for(int x = 1; x < aveIm.cols-1; x++)
        for(int y = 1; y < aveIm.rows-1; y++){
            for(int c=0;c<3;c++){
                dif=0;
                maxDif=0;
                for(int xx=x-1;xx<=x+1;xx++)
                    for(int yy=y-1;yy<=y+1;yy++){
                        dif=abs(aveIm.at<Vec3b>(y,x)[c]-aveIm.at<Vec3b>(yy,xx)[c]);
                        if(dif>maxDif)
                            maxDif=dif;
                    }
                change.at<Vec3b>(y,x)[c]=maxDif;
            }

        }
    for(int x = 1; x < aveIm.cols-1; x++)
        for(int y = 1; y < aveIm.rows-1; y++){
            dif=0;
            for(int c=0;c<3;c++){
                dif+=change.at<Vec3b>(y,x)[c];
            }
            if(dif>5){
                view.at<Vec3b>(y,x)[2]=255;
            }
            if(dif>30){
                view.at<Vec3b>(y,x)[1]=255;
            }
            if(dif>50){
                view.at<Vec3b>(y,x)[0]=255;
            }
        }
    //temporary to invert the image
    for(int x = 0; x < aveIm.cols; x++)
        for(int y = 0; y < aveIm.rows; y++){
            if(view.at<Vec3b>(y,x)[2]==0){
                view.at<Vec3b>(y,x)[2]=255;
                view.at<Vec3b>(y,x)[1]=255;
                view.at<Vec3b>(y,x)[0]=255;
            } else {
                view.at<Vec3b>(y,x)[2]=0;
                view.at<Vec3b>(y,x)[1]=0;
                view.at<Vec3b>(y,x)[0]=0;
            }
        }

    change.release();
    return view;
}

Mat PAOLProcUtils::connectedComponent(Mat& im, bool binary){
    int up,left;
    int x,y;
    Mat out;
    SegList *sList=new SegList();


    out=Mat::zeros(im.size(),im.type());
//add code to only check if it is basically white
    for(x=0;x<im.cols;x++)
        for(y=0;y<im.rows;y++){
            if ((binary &&
                    (im.at<Vec3b>(y,x)[0]!=0 ||
                     im.at<Vec3b>(y,x)[1]!=0 ||
                     im.at<Vec3b>(y,x)[2]!=0)) ||
            (!binary)){

                left=comparePix(out,im,x,y,x-1,y,binary,1,sList);
                up=comparePix(out,im,x,y,x,y-1,binary,1,sList);

                if (left!=0 and up!=0){
                    numToPix(out,x,y,sList->merge(left,up));
                } else if (left!=0){
                    numToPix(out,x,y,sList->find(left));
                } else if (up!=0){
                    numToPix(out,x,y,sList->find(up));
                } else {
                    numToPix(out,x,y,sList->newPoint());
                }
            } else {
                out.at<Vec3b>(y,x)[0]=0;
                out.at<Vec3b>(y,x)[1]=0;
                out.at<Vec3b>(y,x)[2]=0;
            }
        }

    //sList->print();
    sList->update();

    for(x=0;x<out.cols;x++)
        for(y=0;y<out.rows;y++){
            numToPix(out,x,y,sList->getValue(pixToNum(out,x,y)));
        }

    qDebug("countOut=%d",sList->getCount());


    ////my test code
    //sList->zero();
    for(x=0;x<out.cols;x++)
        for(y=0;y<out.rows;y++){
           sList->addCount(pixToNum(out,x,y),im,x,y);
        }
    sList->removeSmallRegions(100);
    sList->removeNoneWhite(10);
    for(x=0;x<out.cols;x++)
        for(y=0;y<out.rows;y++){
            numToPix(out,x,y,sList->getValue(pixToNum(out,x,y)));
        }
    ////ends here


    colorConnected(out,sList);
    sList->~SegList();
    return out;
}

Mat PAOLProcUtils::connectedComponentFlipEliminate(Mat& im,Mat& orig, bool binary){
    int up,left;
    int x,y;
    Mat out;
    SegList *sList=new SegList();


    out=Mat::zeros(im.size(),im.type());

    for(x=0;x<im.cols;x++)
        for(y=0;y<im.rows;y++){
            if ((binary &&
                    (im.at<Vec3b>(y,x)[0]!=0 ||
                     im.at<Vec3b>(y,x)[1]!=0 ||
                     im.at<Vec3b>(y,x)[2]!=0)) ||
            (!binary)){

                left=comparePix(out,im,x,y,x-1,y,binary,1,sList);
                up=comparePix(out,im,x,y,x,y-1,binary,1,sList);

                if (left!=0 and up!=0){
                    numToPix(out,x,y,sList->merge(left,up));
                } else if (left!=0){
                    numToPix(out,x,y,sList->find(left));
                } else if (up!=0){
                    numToPix(out,x,y,sList->find(up));
                } else {
                    numToPix(out,x,y,sList->newPoint());
                }
            } else {
                out.at<Vec3b>(y,x)[0]=0;
                out.at<Vec3b>(y,x)[1]=0;
                out.at<Vec3b>(y,x)[2]=0;
            }
        }

    //sList->print();
    sList->update();

    for(x=0;x<out.cols;x++)
        for(y=0;y<out.rows;y++){
            numToPix(out,x,y,sList->getValue(pixToNum(out,x,y)));
        }

    qDebug("countOut=%d",sList->getCount());


    ////my test code
    //sList->zero();
    for(x=0;x<out.cols;x++){
        y=out.rows-2;
        sList->removeColor(pixToNum(out,x,y));
    }

    for(x=0;x<out.cols;x++)
        for(y=0;y<out.rows;y++){
            numToPix2(out,orig,x,y,sList->getValue(pixToNum(out,x,y)));
        }
    ////ends here


    sList->~SegList();
    return out;
}

int PAOLProcUtils::comparePix(Mat& connected,Mat& data, int x1, int y1, int x2, int y2, bool binary, int thresh, SegList* sList){
    if (x2>=0 && y2>=0){
        if (binary){
            return sList->find(pixToNum(connected,x2,y2));
        } else {
            if (pixSame(data,x1,y1,x2,y2,thresh)){
                return sList->find(pixToNum(connected,x2,y2));
            }else
                return 0;
        }
    } else {
        return 0;
    }
}

bool PAOLProcUtils::pixSame(Mat& im, int x1, int y1, int x2, int y2, int thresh){
    int dif=0;

    for (int c=0;c<3;c++)
        dif+=abs(im.at<Vec3b>(y1,x1)[c]-im.at<Vec3b>(y2,x2)[c]);

    if (dif<=thresh)
        return true;
    else
        return false;
}

int PAOLProcUtils::pixToNum(Mat& im,int x, int y){
    int out;

    out=im.at<Vec3b>(y,x)[0];
    out+=256*im.at<Vec3b>(y,x)[1];
    out+=256*256*im.at<Vec3b>(y,x)[2];

    return out;
}

void PAOLProcUtils::numToPix(Mat& im,int x, int y, int num){
    int zero,one,two;

    zero=num%256;
    one=(num/256)%256;
    two=(num/256)/256;
    im.at<Vec3b>(y,x)[0]=zero;
    im.at<Vec3b>(y,x)[1]=one;
    im.at<Vec3b>(y,x)[2]=two;
}
void PAOLProcUtils::numToPix2(Mat &im, Mat &orig, int x, int y, int num){
    if(num==-1){
        for(int c=0;c<3;c++)
            im.at<Vec3b>(y,x)[c]=0;
    } else {
        for(int c=0;c<3;c++)
            im.at<Vec3b>(y,x)[c]=orig.at<Vec3b>(y,x)[c];
    }
}

void PAOLProcUtils::colorConnected(Mat& connected,SegList* sList){
    int x,y,temp;
    int scale=256*256*256/sList->getCount();

    for(x=0;x<connected.cols;x++)
        for(y=0;y<connected.rows;y++){
            temp=pixToNum(connected,x,y)*scale;
            numToPix(connected,x,y,temp);
        }
}

Mat PAOLProcUtils::minimalDif(Mat& im,int thresh){
    int dif,dif2;
    Mat out;
    out=Mat::zeros(im.size(),im.type());

    for(int x=0;x<im.cols-1;x++)
        for(int y=0;y<im.rows-1;y++){
            dif=0;
            dif2=0;
            for (int c=0;c<3;c++)
                dif+=abs(im.at<Vec3b>(y,x)[c]-im.at<Vec3b>(y+1,x)[c]);
            for (int c=0;c<3;c++)
                dif2+=abs(im.at<Vec3b>(y,x)[c]-im.at<Vec3b>(y,x+1)[c]);

            if(dif>thresh || dif2>thresh){
                out.at<Vec3b>(y,x)[0]=255;
                out.at<Vec3b>(y,x)[1]=255;
                out.at<Vec3b>(y,x)[2]=255;
            }
        }
    return out;
}

Mat PAOLProcUtils::invertToBinary(Mat &im){
    int dif;
    Mat out=Mat::zeros(im.size(),im.type());

    for(int x=0;x<im.cols;x++)
        for(int y=0;y<im.rows;y++){
            dif=0;
            for (int c=0;c<3;c++)
                dif+=im.at<Vec3b>(y,x)[c];
            if(dif==0)
                for (int c=0;c<3;c++)
                    out.at<Vec3b>(y,x)[c]=255;
        }
    return out;
}

Mat PAOLProcUtils::keepWhite(Mat &im,int thresh){
    int rg,gb,br;
    int r,g,b;
    Mat out=im.clone();

    for(int x=0;x<im.cols;x++)
        for(int y=0;y<im.rows;y++){
            r=im.at<Vec3b>(y,x)[2];
            g=im.at<Vec3b>(y,x)[1];
            b=im.at<Vec3b>(y,x)[0];

            br=abs(r-b);
            rg=abs(r-g);
            gb=abs(g-b);
            if(br>thresh ||
                    rg>thresh ||
                    gb>thresh ||
                    r<30){
                for (int c=0;c<3;c++){
                    out.at<Vec3b>(y,x)[c]=0;
                }
            }
        }
    return out;
}




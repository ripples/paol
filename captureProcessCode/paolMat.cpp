#include "paolMat.h"

paolMat::paolMat()
{
  difs = -1;
  name = "No Name"; 
  count = -1;
  cameraNum=-1;
  scale=8;
}

paolMat::paolMat(paolMat* m)
{
    if(m->src.data)
        src = m->src.clone();
    if(m->mask.data)
        mask = m->mask.clone();
    if(m->maskMin.data)
        maskMin = m->maskMin.clone();

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
    scale=m->scale;
    name = m->name;
    difs = m->difs;
    count=m->count;
}

paolMat::~paolMat()
{
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(display.data)
        display.~Mat();
    if(displayMin.data)
        displayMin.~Mat();
    if(maskMin.data)
        maskMin.~Mat();
    if(cam.isOpened())
        cam.release();
}

void paolMat::copy(paolMat *m){
    //clear out existing images and copy in new
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(maskMin.data)
        maskMin.~Mat();

    if(m->src.data)
        src = m->src.clone();
    if(m->mask.data)
        mask = m->mask.clone();
    if(m->maskMin.data)
        maskMin = m->maskMin.clone();

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
    scale=m->scale;
    difs = m->difs;
    name = m->name;
    count=m->count;
}

void paolMat::copyClean(paolMat *m){
    //clear out existing images and make blank
    if(src.data)
        src.~Mat();
    if(mask.data)
        mask.~Mat();
    if(maskMin.data)
        maskMin.~Mat();

    if(m->src.data){
        src = Mat::zeros(m->src.size(),m->src.type());
        mask = Mat::zeros(m->src.size(),m->src.type());
        maskMin = Mat::zeros(m->src.size(),m->src.type());
    }

    cameraNum=m->cameraNum;
    sprintf(readName,"%s",m->readName);
    countRead=m->countRead;
    time=m->time;
    dirOut=m->dirOut;
    scale=m->scale;
}

void paolMat::copyMaskMin(paolMat *m){
    if(maskMin.data)
        maskMin.~Mat();

    if(m->maskMin.data)
        maskMin = m->maskMin.clone();
}

void paolMat::copyMask(paolMat *m){
    if(mask.data)
        mask.~Mat();

    if(m->mask.data)
        mask = m->mask.clone();
}

void paolMat::setCameraNum(int i){
    cam=VideoCapture(i);

    //set parameters for camera
    cam.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
    cam.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);

    //cam.set(CV_CAP_PROP_FRAME_WIDTH, 320);
    //cam.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
}

void paolMat::takePicture(){
    //grab 5 consecutive images to clear camera buffer
    for (int i=0;i<5;i++){
        cam>>src;
    }
}

float paolMat::differenceMin(paolMat *img, int thresh, int size){
    int offset;
    bool diff;
    int numDiff;
    int surroundThresh = 50;

    //maskMin is set to a blank state (1/(scale*scale)) the size of src
    if (!maskMin.data)
        maskMin=Mat::zeros(src.rows/scale,src.cols/scale,src.type());

    if (scale>=size+1)
        offset=scale;
    else
        offset=scale*2;

  numDiff = 0;


  for (int y = offset, yy=offset/scale; y < (src.rows-offset); y+=scale,yy++)
  {
      //for every column
      for (int x = offset, xx=offset/scale; x < (src.cols-offset); x+=scale,xx++)
      {
          diff = false;
          //for each color channel
          for(int i = 0; i < 3; i++)
          {
              //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
              if(abs((double)img->src.at<Vec3b>(y,x)[i]-(double)src.at<Vec3b>(y,x)[i]) > thresh)
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
                          if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>((yy+1),xx)[ii])))>surroundThresh))
                              diff = false;
                          if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>(yy,(xx+1))[ii])))>surroundThresh))
                              diff = false;
                      }
                  }
              }
          }
          if(diff)
          {
              numDiff++;
              maskMin.at<Vec3b>(yy,xx)[2]=255;
          }
      }
  }
  //return the percent of maskMin that are differences
  return (float)numDiff/(float)(maskMin.rows*maskMin.cols);
}

float paolMat::shrinkMaskMin()
{
    int total;
    int numDiff=0;
    //for every 3x3 group of pixels in maskMin
    for(int x = 1; x < maskMin.cols-1; x++)
        for(int y = 1; y < maskMin.rows-1; y++)
        {
            //count the number of differences in the 3x3
            total=0;
            for (int xx=x-1;xx<=x+1;xx++)
                for (int yy=y-1;yy<=y+1;yy++){
                    if(maskMin.at<Vec3b>(yy,xx)[2]==255)
                        total++;
                }
            //if the number of differences is greater then 3 mark it as different and count it
            if(total>3){
                maskMin.at<Vec3b>(y,x)[1]=255;
                numDiff++;
            }
        }
    //return the percentage of the maskMin that are valid differences
    return (float)numDiff/(float)(maskMin.rows*maskMin.cols);
}

//extend differences in maskMin to edges since differences start 1 pixel in
void paolMat::extendMaskMinToEdges(){
    int x;

    //extend bottom and top edge where differences overlap
    for(x=0;x<maskMin.cols;x++){
        if(maskMin.at<Vec3b>(1,x)[1] == 255)
            maskMin.at<Vec3b>(0,x)[1]=255;
        if(maskMin.at<Vec3b>(maskMin.rows-2,x)[1] == 255)
            maskMin.at<Vec3b>(maskMin.rows-1,x)[1]=255;
    }

    //extend right and left edge wher differences overlap
    for(int y = 0; y < maskMin.rows; y++)
    {
        if(maskMin.at<Vec3b>(y,1)[1] == 255)
            maskMin.at<Vec3b>(y,0)[1] = 255;

        if(maskMin.at<Vec3b>(y,maskMin.cols-2)[1] == 255)
            maskMin.at<Vec3b>(y,maskMin.cols-1)[1] = 255;
    }
}

//sweep mask from left, right, and top to extend difference region
void paolMat::sweepDownMin(){
    bool left,right,top;
    //create a temporary Mat the size of maskMin to store results
    Mat temp=Mat::zeros(maskMin.size(),maskMin.type());

    //from left to right
    for(int x = 0; x < maskMin.cols; x++)
    {
        //from top to bottom
        top = false;
        for(int y = 0; y < maskMin.rows; y++){
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                top = true;

            if(top == true)
                temp.at<Vec3b>(y,x)[0] = 255;
        }
    }

    //from top to bottom
    for(int y = 0; y < maskMin.rows; y++){

        //sweep from the left
        left = false;
        for(int x = 0; x < maskMin.cols; x++)
        {
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                left = true;

            if(left == true)
                temp.at<Vec3b>(y,x)[1] = 255;
        }

        //sweep from the right
        right = false;
        for(int x = maskMin.cols-1; x >-1; x--)
        {
            if(maskMin.at<Vec3b>(y,x)[1] == 255)
                right = true;

            if(right == true)
                temp.at<Vec3b>(y,x)[2] = 255;
        }
    }

    if(maskMin.data)
        maskMin.~Mat();
    maskMin = temp.clone();
    temp.~Mat();
}

void paolMat::keepWhiteMaskMin(){
    int x;

    //keep only the white pixels
    for(x = 0; x < maskMin.cols; x++)
    {
        for(int y = 0; y < maskMin.rows; y++)
        {
            if(!(maskMin.at<Vec3b>(y,x)[0] == 255 &&
                 maskMin.at<Vec3b>(y,x)[1] == 255 &&
                 maskMin.at<Vec3b>(y,x)[2] == 255)){
                for (int i=0;i<3;i++)
                    maskMin.at<Vec3b>(y,x)[i]=0;
            }
        }
    }
}

//grow any turned on pixels by size
void paolMat::growMin(int size)
{
    int startx,endx,starty,endy;

    //for every pixel in the image
    for(int y = 0; y < maskMin.rows; y++)
        for(int x = 0; x < maskMin.cols; x++)

            //if the pixel is turned on
            if(maskMin.at<Vec3b>(y,x)[0] == 255){
                startx=x-size;
                if (startx<0)
                    startx=0;

                starty=y-size;
                if (starty<0)
                    starty=0;

                endx=x+size;
                if (endx>=maskMin.cols)
                    endx=maskMin.cols-1;

                endy=y+size;
                if (endy>=maskMin.rows)
                    endy=maskMin.rows-1;

                //grow the region around that pixel
                for(int yy = starty; yy <= endy;yy++)
                    for(int xx = startx; xx <= endx; xx++)
                        maskMin.at<Vec3b>(yy,xx)[1] = 255;
            }
}

void paolMat::findContoursMaskMin(){
    //method combines findCountoursMask and contoursToMask methods

    Mat src_gray;
    int thresh = 100;
    //int max_thresh = 255;
    RNG rng(12345);

    /// Convert image to gray and blur it
    cvtColor( maskMin, src_gray, CV_BGR2GRAY );
    cv::blur( src_gray, src_gray, cv::Size(3,3) );
    // createTrackbar( " Canny thresh:", "Source", &thresh, max_thresh, thresh_callback );

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
    for(int x = 0; x < maskMin.cols; x++)
      for(int y = 0; y < maskMin.rows; y++)
        {
          count=0;
          for(int c=0;c<3;c++)
              count+=drawing.at<Vec3b>(y,x)[c];
          if(count>0)
              maskMin.at<Vec3b>(y,x)[1]=255;
      }
    drawing.~Mat();
    src_gray.~Mat();
    canny_output.~Mat();
}

//method to expand maskMin up to the mask
void paolMat::maskMinToMaskBinary(){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;

    if(mask.data)
        mask.~Mat();
    mask=Mat::zeros(src.size(),src.type());

    //for every color channel
    for (int c=0;c<3;c++){
        //go through the mask scalexscale box
        for (int y = 0; y < mask.rows; y+=scale)
        {
            for (int x = 0; x < mask.cols; x+=scale)
            {
                //set the location on if the corresponding location in maskMin is on
                if (maskMin.at<Vec3b>(y/scale,x/scale)[c]!=0)
                    center=true;
                else
                    center=false;

                //repeat for other corners of box
                if((x+scale)/scale<maskMin.cols){
                    rightIn=true;
                    if(maskMin.at<Vec3b>(y/scale,(x+scale)/scale)[c]!=0)
                        right=true;
                    else
                        right=false;
                } else
                    rightIn=false;

                if((y+scale)/scale<maskMin.rows){
                    downIn=true;
                    if(maskMin.at<Vec3b>((y+scale)/scale,x/scale)[c]!=0)
                        down=true;
                    else
                        down=false;
                } else
                    downIn=false;

                if(downIn && rightIn){
                    //cornerIn=true;
                    if(maskMin.at<Vec3b>((y+scale)/scale,(x+scale)/scale)[c]!=0)
                        corner=true;
                    else
                        corner=false;
                } //else
                    //cornerIn=false;

                //fill in mask based on which corners are turned on based on maskMin
                if(center)
                    mask.at<Vec3b>(y,x)[c]=255;
                if(center || right)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        mask.at<Vec3b>(y,xx)[c]=255;
                if(center || down)
                    for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                        mask.at<Vec3b>(yy,x)[c]=255;
                if(center || right || down || corner)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++)
                            mask.at<Vec3b>(yy,xx)[c]=255;

            }
        }
    }
}

//blur goes through the entire image and makes each pixel the average color of the pixels in a box of radious size around it
void paolMat::blur(int size)
{
    int tempR, tempB, tempG, area;
    //blur size is pixels adjacent i.e. 1 would be a 3x3 square centered on each pixel
    area = (size *2+1)*(size *2+1);

    paolMat *temp;
    temp = new paolMat(this);
    temp->src = Scalar(0,0,0);

    //for all possible locations in the image
    for(int y = size; y < mask.rows - size; y++)
        for(int x = size; x < mask.cols -size; x++)
        {
            //average the pixel color of the surrounding box
            tempB = 0;
            tempG = 0;
            tempR = 0;
            for(int yy = y-size; yy <= y+size; yy++)
                for(int xx = x-size; xx <= x+size; xx++)
                {
                    tempR+=src.at<Vec3b>(yy,xx)[2];
                    tempG+=src.at<Vec3b>(yy,xx)[1];
                    tempB+=src.at<Vec3b>(yy,xx)[0];
                }
            tempR /=area;
            tempG /=area;
            tempB /=area;
            //set the pixel color
            temp->src.at<Vec3b>(y,x)[2] = tempR;
            temp->src.at<Vec3b>(y,x)[1] = tempG;
            temp->src.at<Vec3b>(y,x)[0] = tempB;

        }
    if(src.data)
        src.~Mat();
    src = temp->src.clone();
    temp->~paolMat();
}

//this method seeks to identify where possible content is in the src image
void paolMat::pDrift()
{
    int temp,total;
    //clean mask
    if (mask.data){
        mask.~Mat();
    }
    mask=Mat::zeros(src.size(),src.type());

    //for every pixel in image (excludeing edges where perocess would break
    for(int y = 0; y < src.rows -1; y++)
        for(int x = 0; x < src.cols -1; x++)
        {
            //look for edges in the vertical direction using a variation on a Sobel filter
            //[1 -1]
            temp = (
                        //y,x+1
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y,x+1)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y,x+1)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y,x+1)[2])
                    );

            if(temp > 255)
                temp = 255;
            //write the vertical edge information to the red color channel
            mask.at<Vec3b>(y,x)[2] = temp;
            total = temp;

            //run the same filters in the vertical direction to look for edges in the
            //horizontal direction.
            temp = (
                        //y+1,x
                        abs(src.at<Vec3b>(y,x)[0] - src.at<Vec3b>(y+1,x)[0])+
                    abs(src.at<Vec3b>(y,x)[1] - src.at<Vec3b>(y+1,x)[1])+
                    abs(src.at<Vec3b>(y,x)[2] - src.at<Vec3b>(y+1,x)[2])
                    );
            if(temp > 255)
                temp = 255;
            total+=temp;
            if(total > 255)
                total = 255;

            //write the horizontal edge information to the green color channel
            mask.at<Vec3b>(y,x)[1] = temp;
            //write the addition of the horizontal and vertical edges found to the blue color channel
            mask.at<Vec3b>(y,x)[0] = total;
        }
}

//grow the areas highlighted in the 0 color channel of the mask
// //translation-if the difference in blue is enough then turn on the surrounding red pixel
void paolMat::grow(int blueThresh, int size)
{
    //for every pixel
    for(int y = size; y < src.rows - size; y++)
        for(int x = size ; x < src.cols - size; x++)
            //if the value in the mask is greater then a theshold
            if(mask.at<Vec3b>(y,x)[0] > blueThresh){
                //brighten edge
                mask.at<Vec3b>(y,x)[0]=255;
                //turn all the pixels in the square of size 2*size+1 around it on in the 2 color channel
                for(int yy = y-size; yy <= y+size;yy++)
                    for(int xx = x-size; xx <= x+size; xx++)
                        mask.at<Vec3b>(yy,xx)[2] = 255;
            } else {
                mask.at<Vec3b>(y,x)[0]=0;
            }
}

void paolMat::nontextToWhite()
{
    int x,y;
    for(y = 0; y < mask.rows; y++)
        for(x = 0; x < mask.cols; x++){
            if(mask.at<Vec3b>(y,x)[2] < 255){
                src.at<Vec3b>(y,x)[0] = 255;
                src.at<Vec3b>(y,x)[1] = 255;
                src.at<Vec3b>(y,x)[2] = 255;
            }
        }
    //blank the first row because it had bad information
    for(y = 0; y < mask.rows; y++){
        x=0;
        src.at<Vec3b>(y,x)[0] = 255;
        src.at<Vec3b>(y,x)[1] = 255;
        src.at<Vec3b>(y,x)[2] = 255;
    }
}

//updates pixels inthe background image where there is no foreground in maskMin
void paolMat::updateBackgroundMaskMin(paolMat *m, paolMat *foreground){
    bool center,right,down,corner;
    bool rightIn,downIn;//,cornerIn;
    int count=0;

    //go through the entire image
    for (int y = 0; y < src.rows; y+=scale)
    {
        for (int x = 0; x < src.cols; x+=scale)
        {
            //check the maskMin to see where changes are and upscale them to size of src
            if (m->maskMin.at<Vec3b>(y/scale,x/scale)[0]!=0)
                center=true;
            else
                center=false;

            //find boundaries
            if((x+scale)/scale<maskMin.cols){
                rightIn=true;
                if(m->maskMin.at<Vec3b>(y/scale,(x+scale)/scale)[0]!=0)
                    right=true;
                else
                    right=false;
            } else
                rightIn=false;

            if((y+scale)/scale<maskMin.rows){
                downIn=true;
                if(m->maskMin.at<Vec3b>((y+scale)/scale,x/scale)[0]!=0)
                    down=true;
                else
                    down=false;
            } else
                downIn=false;

            if(downIn && rightIn){
                //cornerIn=true;
                if(m->maskMin.at<Vec3b>((y+scale)/scale,(x+scale)/scale)[0]!=0)
                    corner=true;
                else
                    corner=false;
            } //else
            //cornerIn=false;

            //if there is nothing in the mask (no change hense no foreground) update the pixel
            for(int c=0;c<3;c++){
                if(!center){
                    src.at<Vec3b>(y,x)[c]=foreground->src.at<Vec3b>(y,x)[c];
                    count++;
                }
                if(!center && !right){
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++){
                        src.at<Vec3b>(y,xx)[c]=foreground->src.at<Vec3b>(y,xx)[c];
                        count++;
                    }
                }
                if(!center && !down){
                    for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++){
                        src.at<Vec3b>(yy,x)[c]=foreground->src.at<Vec3b>(yy,x)[c];
                        count++;
                    }
                }

                if(!center && !right && !down && !corner)
                    for(int xx=x+1; xx<mask.cols && xx<x+scale;xx++)
                        for(int yy=y+1; yy<mask.rows && yy<y+scale; yy++){
                            src.at<Vec3b>(yy,xx)[c]=foreground->src.at<Vec3b>(yy,xx)[c];
                            count++;
                        }
            }
        }
    }
}

//this method updates the whiteboard image, removing the professor
//the foreground image contains the foreground information in src and the
//  information on movement in its mask
//the edgeInfo image contains the current mask data related to edges
void paolMat::updateBack2(paolMat *foreground,paolMat *edgeInfo){
    //if no mask exists (the first time) create a blank mask
    if(!mask.data)
        mask=Mat::zeros(src.size(),src.type());

    //for every pixel in the image
    for (int y = 0; y < src.rows; y++)
    {
        for (int x = 0; x < src.cols; x++)
        {
            //if there was no movement at that pixel
            if (foreground->mask.at<Vec3b>(y,x)[0]==0){
                //update what the whiteboard and edge information look like at that pixel
                for (int c=0;c<3;c++){
                    src.at<Vec3b>(y,x)[c]=foreground->src.at<Vec3b>(y,x)[c];
                    mask.at<Vec3b>(y,x)[c]=edgeInfo->mask.at<Vec3b>(y,x)[c];
                }
            }
        }
    }
}

//fix to work on a separate image so that it doesn't kill background
void paolMat::processText(paolMat *m){
    int start,end;
    int dif;
    int r,g,b;
    int rAve,gAve,bAve;
    int count;
    int rOut,gOut,bOut;

    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++)
            if( m->src.at<Vec3b>(y,x)[0] !=255 ||
                    m->src.at<Vec3b>(y,x)[1] !=255 ||
                    m->src.at<Vec3b>(y,x)[2] !=255 ){
                start = x;//set start of section at first non-white
                r=0;
                g=0;
                b=0;
                count=0;

                //for all pixels until next white pixel
                for(;x < src.cols && ( m->src.at<Vec3b>(y,x)[0] !=255 ||
                                       m->src.at<Vec3b>(y,x)[1] !=255 ||
                                       m->src.at<Vec3b>(y,x)[2] !=255 ); x++){
                    end = x;//reset end of section
                    r += m->src.at<Vec3b>(y,x)[2];
                    g += m->src.at<Vec3b>(y,x)[1];
                    b += m->src.at<Vec3b>(y,x)[0];
                    count++;
                }

                rAve=r/count;
                gAve=g/count;
                bAve=b/count;

                for(int xx = start; xx <= end; xx++){
                    dif=abs(m->src.at<Vec3b>(y,xx)[2]-rAve);
                    dif+=abs(m->src.at<Vec3b>(y,xx)[1]-gAve);
                    dif+=abs(m->src.at<Vec3b>(y,xx)[0]-bAve);

                    if(dif<75){
                        rOut = 255;
                        gOut = 255;
                        bOut = 255;
                    }else{
                        rOut = rAve + (m->src.at<Vec3b>(y,xx)[2]-rAve)*2;
                        gOut = gAve + (m->src.at<Vec3b>(y,xx)[1]-gAve)*2;
                        bOut = bAve + (m->src.at<Vec3b>(y,xx)[0]-bAve)*2;

                        if(rOut > 255)
                            rOut = 255;
                        else if(rOut < 0)
                            rOut = 0;

                        if(gOut > 255)
                            gOut = 255;
                        else if(gOut < 0)
                            gOut = 0;

                        if(bOut > 255)
                            bOut = 255;
                        else if(bOut < 0)
                            bOut = 0;
                    }
                    src.at<Vec3b>(y,xx)[0] = bOut;
                    src.at<Vec3b>(y,xx)[1] = gOut;
                    src.at<Vec3b>(y,xx)[2] = rOut;
                }
            } else {
                src.at<Vec3b>(y,x)[0] = 255;
                src.at<Vec3b>(y,x)[1] = 255;
                src.at<Vec3b>(y,x)[2] = 255;
            }
}

//method for darkening text and setting whiteboard to white
void paolMat::darkenText(){
    //int temp;
    Mat tempOut;

    //for every pixel
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++){
            //write edge information from blue into green channel and zero out red
            mask.at<Vec3b>(y,x)[1]=mask.at<Vec3b>(y,x)[0];
            if (mask.at<Vec3b>(y,x)[1]>15)
                mask.at<Vec3b>(y,x)[1]=255;
            mask.at<Vec3b>(y,x)[2]=0;
        }

    //run a morphological closure (grow then shrink)
    //this will fill in spaces in text caused by only looking at edges
    int dilation_type = MORPH_RECT;
    int dilation_size = 1;
    Mat element = getStructuringElement( dilation_type,
                                           Size( 2*dilation_size + 1, 2*dilation_size+1 ),
                                           Point( dilation_size, dilation_size ) );

    dilate(mask, tempOut, element);
    erode(tempOut, tempOut, element);

    //for every pixel
    for(int y = 0; y < src.rows; y++)
        for(int x = 0; x < src.cols; x++){
            //code to make it look pretty on the mask
            if (mask.at<Vec3b>(y,x)[1]!=255 && tempOut.at<Vec3b>(y,x)[1]>50){
                mask.at<Vec3b>(y,x)[2]=255;
            }

            //if there isn't and edge (text) in that location turn the pixel white
            if (tempOut.at<Vec3b>(y,x)[1]<50){
                src.at<Vec3b>(y,x)[0]=255;
                src.at<Vec3b>(y,x)[1]=255;
                src.at<Vec3b>(y,x)[2]=255;
            }
        }
}

//method to determine group truth of what it whiteboard in the image
//averages the brightest 25% of pixels in each square of size size
void paolMat::averageWhiteboard(int size){
    int x,y,xx,yy;
    int count,color,thresh;
    vector <int> pix;
    vector <int> ave;

    //clear the mask and create a new one
    if(mask.data)
        mask.~Mat();
    mask=Mat::zeros(src.size(),src.type());

    //go through the image by squares of radius size
    for (x=0;x<src.cols;x+=size)
        for (y=0;y<src.rows;y+=size){
            pix.clear();
            ave.clear();

            //within each square create a vector pix that hold all brightness values
            //for the pixels
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=src.at<Vec3b>(yy,xx)[c];
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
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    color=0;
                    for (int c=0;c<3;c++)
                        color+=src.at<Vec3b>(yy,xx)[c];
                    color/=3;

                    if(color>=thresh){
                        count++;
                        for (int c=0;c<3;c++)
                            ave[c]+=src.at<Vec3b>(yy,xx)[c];
                    }
                }
            //figure out the average brightness of each channel for the brightest pixels
            for (int c=0;c<3;c++)
                ave[c]/=count;

            //set the pixels in the mask to the average brightness of the image, square by square
            for(xx=x; xx<x+size && xx<src.cols; xx++)
                for (yy=y; yy<y+size && yy<src.rows; yy++){
                    for (int c=0;c<3;c++)
                        mask.at<Vec3b>(yy,xx)[c]=ave[c];
                }
        }
}

//Use the average image to turn the whiteboard of the image white and to darken the text
void paolMat::enhanceText(){
    int dif;

    //for every pixel in the image and for every color channel
    for(int x = 0; x < src.cols; x++)
        for(int y = 0; y < src.rows; y++){
            for(int c=0;c<3;c++){

                //if the pixel is not 0 (just put in so that we don't divide by 0)
                if (mask.at<Vec3b>(y,x)[c]>0){
                    //take the brightness of the pixel and divide it by what white is in
                    //that location (average from mask)
                    dif=255*src.at<Vec3b>(y,x)[c]/mask.at<Vec3b>(y,x)[c];
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
                src.at<Vec3b>(y,x)[c]=dif;
            }
        }
}

//gives the percentage of differences in text in the image
// it does this by counting the number of times text (0-blue color channel)
// is the not the same between images and where it appears in one image is not in an area surrounding text
// in the other
float paolMat::countDifsMask(paolMat *newIm){
    int difs=0;

    for(int x = 0; x < src.cols; x++)
        for(int y = 0; y < src.rows; y++){
            if((mask.at<Vec3b>(y,x)[0]!=0 and newIm->mask.at<Vec3b>(y,x)[2]!=255) ||
                (newIm->mask.at<Vec3b>(y,x)[0]!=0 and mask.at<Vec3b>(y,x)[2]!=255)){
                difs++;
                mask.at<Vec3b>(y,x)[0]=255;
                mask.at<Vec3b>(y,x)[1]=255;
                mask.at<Vec3b>(y,x)[2]=255;
            } else {
                mask.at<Vec3b>(y,x)[0]=0;
                mask.at<Vec3b>(y,x)[1]=0;
                mask.at<Vec3b>(y,x)[2]=0;
            }
        }

    return (double)difs/((double)(mask.cols*mask.rows));
}

void paolMat::difference(paolMat *img, int thresh, int size, int maskBottom)
{
  bool diff;
  int numDiff;
  int surroundThresh = 50;
  int dist;
  bool first;
  int cenx;
  int ceny;
  int total;
  //mask is set to a blank state
  mask = Mat::zeros(mask.size(), mask.type());
  
  numDiff = 0;
  first = true;
  //distance --
  dist = 0;
  //for every row
  for (int y = size; y < (src.rows-(size+1+maskBottom)); y++)
    {	
      //for every column
      for (int x = size; x < (src.cols-(size+1)); x++)
	{
	  diff = false;
	  //for each color channel
	  for(int i = 0; i < 3; i++)
	    {
	      //if the difference (for this pixel) between the the current img and the previous img is greater than the threshold, difference is noted; diff = true
	      if(abs((double)img->src.at<Vec3b>(y,x)[i]-(double)src.at<Vec3b>(y,x)[i])>thresh)
		diff = true;
	    }
	  if(diff)
	    {
	      //std::cout<<"First if dif size: "<<size<<std::endl;
	      //mask.at<Vec3b>(y,x)[1]=255;
	      // for all the pixels surrounding the current pixel
	      for(int yy = y-size; yy < y+size; yy++)
		{
		  for(int xx = x-size; xx < x+size; xx++)
		    {
		      //for each color channel
		      for(int ii = 0; ii < 3; ii++)
			{
			  //ignore all differneces found at the edges; sometimes pixels get lost in tranmission
			  if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>((yy+1),xx)[ii])))>surroundThresh))
			    diff = false;
			  if(abs(((double)(img->src.at<Vec3b>(yy,xx)[ii]))-(((double)(img->src.at<Vec3b>(yy,(xx+1))[ii])))>surroundThresh))
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
	      total = abs((double)img->src.at<Vec3b>(y,x)[0]-(double)src.at<Vec3b>(y,x)[0]) +
		abs((double)img->src.at<Vec3b>(y,x)[1]-(double)src.at<Vec3b>(y,x)[1]) +
		abs((double)img->src.at<Vec3b>(y,x)[2]-(double)src.at<Vec3b>(y,x)[2]);
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
  //std::cout<<"Difference dist: "<<dist<<std::endl;
  if((dist<10000))//&&(maskBottom>0))
    difs = 0;
  else
    difs = numDiff;
}

void paolMat::read(std::string fullName, std::string fileName,int countIn, int timeIn)
{
  name = fileName;
  src = imread(fullName);
  mask = Mat::zeros(src.size(), src.type());
  count=countIn;
  time=timeIn;
  //if(src.data)
  //std::cout<<"PaolMat:: Read: "<<fullName<<std::endl;
}

void paolMat::write2(std::string outDir,std::string nameOut,int camNum)
{
  if(!src.empty())
    {
      char temp[256];
      std::string longName = outDir;
      longName.append(nameOut);
      sprintf(temp,"%010d-%d.png",time,camNum);
      longName.append(temp);
      cv::imwrite(longName, src);
      //std::cout<<longName<<std::endl;
    }else
    {
      std::cout<<"   Tried to write a empty src"<<std::endl;
    }
}

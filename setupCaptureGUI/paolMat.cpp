#include "paolMat.h"

paolMat::paolMat(){
    cameraNum=-1;
    scale=8;
    difs = -1;
}

paolMat::paolMat(paolMat* m){
    difs = -1;

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
}

paolMat::paolMat(Mat m, int camIn){
    difs = -1;

    if(m.data)
        src = m.clone();

    cameraNum= camIn;
    scale=8;
}

paolMat::~paolMat(){
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

bool paolMat::readNext(QWidget *fg){
    std::string readFileName;
    int lastLoaded, lastCountRead;
    int lastRead;
    int numberImagesTest = 300;
    int timeCheckOver = 300;

    //if no image has yet been loaded open a dialog to let the user select the first image
    if (cameraNum==-1){
        //open dialog window to find first image, return file name and path
        QString filename=QFileDialog::getOpenFileName(fg,fg->tr("Open First Image of Sequence"),".",
                                                      fg->tr("Image Files (*.png *.bmp *.jpg *.JPG)"));
        //split the filename into image name and directory path
        QStringList pieces = filename.split( "/" );
        QString imagename = pieces.value( pieces.length() - 1 );
        QString directory = pieces[0]+"";
        for (int i=0;i<pieces.length()-1;i++){
            directory+=pieces[i]+"/";
        }
        //convert filename and directory to std strings
        dirOut=directory.toUtf8().constData();
        std::string text = imagename.toUtf8().constData();
        //read in image name and information
        sscanf(text.c_str(),"%[^0-9]%06d-%10d-%d.png",readName,&countRead,&time,&cameraNum);
    }

    //increase image number and create new image name
    countRead++;
    char tempOut[256];
    sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);

    qDebug("readName:%s \n fullPath:%s\n",readName,tempOut);
    //clear src mat and try to read image name
    if(src.data)
        src.~Mat();
    src=imread(tempOut,CV_LOAD_IMAGE_COLOR);

    lastRead=countRead;

    //if image read failed
    if(!src.data){
        //store the time and number of last image read
        lastLoaded=time;
        lastCountRead=countRead;
        //while the count is less then the maximum count between images and no image has been loaded
        while((countRead-lastCountRead)<numberImagesTest && !src.data){
            //reset time to that of last loaded
            time=lastLoaded;
            //while the time is less then the maximum time between images and no image as been loaded
            while((time-lastLoaded)<timeCheckOver && !src.data){
                //increment time
                time++;
                //create new filename
                sprintf(tempOut,"%s%s%06d-%10d-%d.png",dirOut.c_str(),readName,countRead,time,cameraNum);
                //try to load filename
                src=imread(tempOut,CV_LOAD_IMAGE_COLOR);
                lastRead=countRead;
            }
            countRead++;
        }
    }
    //qDebug("rows=%d cols=%d",(int)src.rows,(int)src.cols);

    countRead=lastRead;
    if(!src.data){
        return false;
    } else {
        return true;
    }
}

QImage paolMat::convertToQImage(){
    cvtColor(src,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);

    return img;
}

QImage paolMat::convertMaskToQImage(){
    //copy mask Mat to display Mat and convert from BGR to RGB format
    cvtColor(mask,display,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(display.data),display.cols,display.rows,display.step,QImage::Format_RGB888);

    return img;
}

QImage paolMat::convertMaskMinToQImage(){
    //copy maskMin Mat to displayMin Mat and convert from BGR to RGB format
    cvtColor(maskMin,displayMin,CV_BGR2RGB);

    //convert Mat to QImage
    QImage img=QImage((const unsigned char*)(displayMin.data),displayMin.cols,displayMin.rows,displayMin.step,QImage::Format_RGB888);

    return img;
}

void paolMat::displayImage(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertToQImage();
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void paolMat::displayMask(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMaskToQImage();
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
}

void paolMat::displayMaskMin(QLabel &location){
    //call method to convert Mat to QImage
    QImage img=convertMaskMinToQImage();
    //push image to display location "location"
    location.setPixmap(QPixmap::fromImage(img));
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
  mask = Mat::zeros(src.size(), src.type());

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
    //qDebug("count=%d",count);
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
    int sizeBuffer=40;//area around edge of whiteboard that is ignored when looking for difference
    //there is often a lot of noise at the edges

    for(int x = sizeBuffer; x < src.cols-sizeBuffer; x++)
        for(int y = sizeBuffer; y < src.rows-sizeBuffer; y++){
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
            //src.at<Vec3b>(y,x)[2]=255;
        }

    return (double)difs/((double)(mask.cols*mask.rows));
}

void paolMat::rectifyImage(paolMat *m){
    double widthP,heightP;
    double LTx,LTy,LBx,LBy,RTx,RTy,RBx,RBy;//L left R right T top B bottom
    LTx=547;
    LTy=290;
    LBx=527;
    LBy=932;
    RTx=1831;
    RTy=222;
    RBx=1914;
    RBy=904;
    int xInput,yInput;
    double LPx,LPy,RPx,RPy;//end points of line between edges on which point is found
    /*int rows,cols;
    if (RTx-LTx>RBx-LBx)
        cols=RTx-LTx;
    else
        cols=RBx-LBx;
    if (RBy-RTy>LBy-LTy)
        rows=RBy-RTy;
    else
        rows=LBy-LTy;
*/
    if(src.data)
        src.~Mat();
    //src=Mat::zeros(rows,cols,m->src.type());
    src=Mat::zeros(m->src.size(),m->src.type());

    for(int x = 0; x < src.cols; x++)
        for(int y = 0; y < src.rows; y++){
            widthP=(double)x/(double)src.cols;
            heightP=(double)y/(double)src.rows;
            LPx=LTx+(LBx-LTx)*heightP;
            LPy=LTy+(LBy-LTy)*heightP;
            RPx=RTx+(RBx-RTx)*heightP;
            RPy=RTy+(RBy-RTy)*heightP;

            xInput=(int)(LPx+(RPx-LPx)*widthP);
            yInput=(int)(LPy+(RPy-LPy)*widthP);

            if (xInput>=0 &&
                    xInput<m->src.cols &&
                    yInput>=0 &&
                    yInput<m->src.rows){
                src.at<Vec3b>(y,x)[0]=m->src.at<Vec3b>(yInput,xInput)[0];
                src.at<Vec3b>(y,x)[1]=m->src.at<Vec3b>(yInput,xInput)[1];
                src.at<Vec3b>(y,x)[2]=m->src.at<Vec3b>(yInput,xInput)[2];
            } else {
                src.at<Vec3b>(y,x)[0]=0;
                src.at<Vec3b>(y,x)[1]=0;
                src.at<Vec3b>(y,x)[2]=0;
            }
        }
}

void paolMat::findBoard(paolMat *m){
    Canny(m->src, mask, 50, 200, 3);
    cvtColor(mask, src, CV_GRAY2BGR);

    /*vector<Vec4i> lines;
    HoughLinesP( mask, lines, 1, CV_PI/180, 80, 30, 10 );
    for( size_t i = 0; i < lines.size(); i++ )
    {
        line( src, Point(lines[i][0], lines[i][1]),
                Point(lines[i][2], lines[i][3]), Scalar(0,0,255), 3, 8 );
    }*/


    vector<Vec2f> lines;
    // detect lines
    HoughLines(mask, lines, 1, CV_PI/180, 250, 0, 0 );
    src.~Mat();
    src=m->src.clone();
    // draw lines
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
        line( src, pt1, pt2, Scalar(0,0,255), 3, CV_AA);
    }
}

void paolMat::Standard_Hough(int, void*){
    vector<Vec2f> s_lines; //lines from hough
    //==============================
    //INIT SET UP
    Size s = src.size();
    int maxY = s.height;
    int maxX = s.width;

    //0 is tl, 1 is tr, 2 is br, 3 is bl
    int corners[4][2];
    corners[0][0]=0;
    corners[0][1]=0;
    corners[1][0]=maxX;
    corners[1][1]=0;
    corners[2][0]=maxX;
    corners[2][1]=maxY;
    corners[3][0]=0;
    corners[3][1]=maxY;

    //set centers
    int centX = (maxX / 2);
    int centY = (maxY / 2);
    //END OF INIT SETUP
    //===============================

    Mat src_edited;
    Mat src_crop;
    Mat standard_hough;
    int x, y, width, height;

    cvtColor(src, src_edited, CV_RGB2GRAY);//COLOR_RG
    Canny(src_edited, src_edited, 50, 200, 3);//EDGES

    cvtColor(src_edited, standard_hough, COLOR_GRAY2BGR); //make edges gray
    HoughLines(src_edited, s_lines, 1, CV_PI / 180, 180, 0, 0);

    //loop through each line and get points of end to end of lines and find closest to center
    for (size_t i = 0; i < s_lines.size(); i++){
        float r = s_lines[i][0], t = s_lines[i][1];
        double cos_t = cos(t), sin_t = sin(t);
        double x0 = r*cos_t, y0 = r*sin_t;
        double alpha = 1000;

        Point pt1(cvRound(x0 + alpha*(-sin_t)), cvRound(y0 + alpha*cos_t));
        Point pt2(cvRound(x0 - alpha*(-sin_t)), cvRound(y0 - alpha*cos_t));
        line(standard_hough, pt1, pt2, Scalar(255, 0, 0), 3, CV_AA);

        //if pt1 is bottom point switch to top
        if (pt1.y > pt2.y){
            Point tempP = pt2;
            pt2 = pt1;
            pt1 = tempP;
        }

        float degree = atan2((pt2.x - pt1.x), (pt2.y - pt1.y)) * (180 / M_PI);

        //make sure its a viable line, may need to be altered
        if (abs(degree) > 70 && abs(degree) < 110){
            if (pt1.x >= corners[0][0] && pt1.x < centX){
                corners[0][0] = pt1.x;
                corners[3][0] = pt2.x;
            }
            if (pt1.x <= corners[1][0] && pt1.x > centX){
                corners[1][0] = pt1.x;
                corners[2][0] = pt2.x;
            }
        }

        //make sure its a viable line, may need to be altered
        if (abs(degree) < 20 || abs(degree) > 160){
            if (pt1.y >= corners[0][1] && pt1.y < centY){
                corners[0][1] = pt1.y;
                corners[1][1] = pt2.y;
            }

            if (pt1.y <= corners[3][1] && pt1.y > centY){
                corners[3][1] = pt1.y;
                corners[2][1] = pt2.y;
            }
        }
    }

    if (corners[0][0] < corners[3][0]){
        x = corners[0][0];
    }
    else{
        x = corners[3][0];
    }

    if (corners[1][0] > corners[2][0]){
        width = corners[1][0] - x;
    }
    else{
        width = corners[2][0] - x;
    }

    if (corners[0][1] < corners[1][1]){
        y = corners[0][1];
    }
    else{
        y = corners[1][1];
    }

    if (corners[3][1] > corners[2][1]){
        height = corners[3][1] - y;
    }
    else{
        height = corners[2][1] - y;
    }
    Rect xRect(x, y, width, height);
    src = src(xRect); //This is cropped to board
    //corners should still be coords of rect (could probably be saved off in a different for like x,y,w,h instead of 4 points)
    string outputCorners= "";

    for(int i = 0; i <= 3; i++){
        for(int j = 0; j < 2; j++){
            string s;
            //string stream used to convert int to string
            stringstream out;
            out << corners[i][j];
            s = out.str();
            outputCorners = outputCorners + s + "\n";
        }
    }
    string camNumStr;
    stringstream numIn;
    numIn << cameraNum;
    camNumStr = numIn.str();

    string path = "/home/paol/paol-code/cornersCam" + camNumStr + ".txt";

    char * writeable = new char[path.size() + 1];

    std::copy(path.begin(), path.end(), writeable);
    writeable[path.size()] = '\0';

    std::ofstream file(writeable);
    file << outputCorners;

 }

// Locate the marker strokes with the DoG and connected components approach.
// First, find the DoG edges and threshold them. Then, use pDrift to determine
// which connected components from the threholded DoG edges should be kept.
Mat paolMat::findMarkerWithCC(const Mat& orig) {
    Mat markerCandidates = getDoGEdges(orig, 13, 17, 1);
    markerCandidates = adjustLevels(markerCandidates, 0, 4, 1);
    markerCandidates = binarize(markerCandidates, 10);
    Mat markerLocations = pDrift(orig);
    markerLocations = binarize(markerLocations, 10);
    return filterConnectedComponents(markerCandidates, markerLocations);
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
Mat paolMat::getDoGEdges(const Mat& orig, int kerSize, float sig1, float sig2) {
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
Mat paolMat::adjustLevels(const Mat& orig, int lo, int hi, double gamma) {
    return 255/pow(hi-lo, 1/gamma)*(orig-lo)^(1/gamma);
}

// Determine the pixels in orig where all the values of all three
// channels are greater than the threshold
Mat paolMat::binarize(const Mat& orig, int threshold) {
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

// Run a specialized edge filter on the original image. The derivative in the horizontal
// direction is stored in the red channel (for vertical edges), the derivative in the
// vertical direction is stored in the green channel (for horizontal edges), and the
// sum is stored in the blue channel
Mat paolMat::pDrift(const Mat& orig) {
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

// Given a binary image, locate the connected components in the image
// WARNING: The returned array must be manually destroyed after use.
int** paolMat::getConnectedComponents(const Mat& components) {
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
Mat paolMat::filterConnectedComponents(const Mat& compsImg, const Mat& keepCompLocs) {
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

// Given a whiteboard image and the marker locations, make the non-marker pixels white
// Arguments:
//    whiteboardImg: The whiteboard image to modify
//    markerPixels: A binary image where the marker pixel locations are white
Mat paolMat::whitenWhiteboard(const Mat& whiteboardImg, const Mat& markerPixels) {
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

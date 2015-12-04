#include "seglist.h"

SegList::SegList()
{
    ccount=0;
    rcount=-1;
    sList.push_back(0);
    pixCount.push_back(0);
    red.push_back(0);
    green.push_back(0);
    blue.push_back(0);
}

SegList::~SegList(){
    sList.clear();
    pixCount.clear();
    red.clear();
    green.clear();
    blue.clear();
}

int SegList::find(int x){
    if (sList[x]==x)
        return x;
    else {
        sList[x]=find(sList[x]);
        return sList[x];
    }
}

int SegList::merge(int u, int l){
    int uv,lv;

    uv=find(u);
    lv=find(l);
    if(uv<lv){
        sList[lv]=uv;
        return uv;
    } else {
        sList[uv]=lv;
        return lv;
    }
}

int SegList::newPoint(){
    ccount++;
    sList.push_back(ccount);
    pixCount.push_back(0);
    red.push_back(0);
    green.push_back(0);
    blue.push_back(0);
    return ccount;
}

void SegList::update(){
    rcount=0;
    unsigned int x;
    for (x=1;x<sList.size();x++){
        if(sList[x]==x){
            rcount++;
            sList[x]=rcount;
        } else {
            sList[x]=sList[sList[x]];
        }
    }
}

void SegList::print(){
    for (unsigned int x=0;x<sList.size();x++){
        qDebug("[%d]->%d",x,sList[x]);
    }
}

int SegList::getCount(){
    return rcount;
}

int SegList::getValue(int x){
    return sList[x];
}

void SegList::zero(){
    for (unsigned int x=0;x<sList.size();x++){
        pixCount[x]=0;
    }
}
void SegList::addCount(int loc,Mat& im,int x,int y){
    pixCount[loc]++;
    red[loc]+=im.at<Vec3b>(y,x)[2];
    green[loc]+=im.at<Vec3b>(y,x)[1];
    blue[loc]+=im.at<Vec3b>(y,x)[0];
}
void SegList::removeSmallRegions(int thresh){
    for (unsigned int x=0;x<pixCount.size();x++){
        if(pixCount[x]<thresh){
            pixCount[x]=0;
            sList[x]=0;
        }
    }
}
void SegList::removeNoneWhite(int thresh){
    int rg,gb,br;
    int r,g,b;
    for (unsigned int x=0;x<pixCount.size();x++){
        if(pixCount[x]!=0){
            /*if(pixCount[x]>100000){
                qDebug("r:%f g:%f b:%f count:%d",red[x],green[x],blue[x],pixCount[x]);
            }*/
            r=(int)(red[x]/pixCount[x]);
            g=(int)(green[x]/pixCount[x]);
            b=(int)(blue[x]/pixCount[x]);
            /*if(pixCount[x]>100000){
                qDebug("r:%d g:%d b:%d count:%d",r,g,b,pixCount[x]);
            }*/
            br=abs(r-b);
            rg=abs(r-g);
            gb=abs(g-b);
            if(br>thresh ||
                    rg>thresh ||
                    gb>thresh ||
                    r<50){
                pixCount[x]=0;
                sList[x]=0;
            }
        }
    }
}

void SegList::removeColor(int color){
    //qDebug("color=%d",color);
    if(sList[color]!=0){
        pixCount[color]=0;
        sList[color]=-1;
    }
}

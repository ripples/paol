#include "clickable_label.h"

clickable_label::clickable_label(QWidget *parent) :
    QLabel(parent)
{
}

void clickable_label::mouseMoveEvent(QMouseEvent *ev){
    this->x = ev->x();
    this->y = ev->y();
    emit Mouse_Pos();
}

void clickable_label::mousePressEvent(QMouseEvent *ev){
    emit Mouse_Pressed();
}

void clickable_label::leaveEvent(QMouseEvent *ev){
    emit Mouse_Left();
}

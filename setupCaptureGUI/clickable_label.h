#ifndef CLICKABLE_LABEL_H
#define CLICKABLE_LABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QDebug>

class clickable_label : public QLabel
{
    Q_OBJECT
public:
    explicit clickable_label(QWidget *parent = 0);

    void mouseMoveEvent(QMouseEvent *ev);
    void mousePressEvent(QMouseEvent *ev);
    void leaveEvent(QMouseEvent *ev);

    int x, y;

signals:
    void Mouse_Pressed();
    void Mouse_Pos();
    void Mouse_Left();

public slots:

};

#endif // CLICKABLE_LABEL_H

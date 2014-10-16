//ALU object definition
//Uses no control code, simply a useful reference for QT to draw and get QPoints for them
//http://qt-project.org/doc/qt-4.8/gettingstartedqt.html

#ifndef __ALU__
#define __ALU__

#include <QWidget>
#include <QPolygon>
#include <QPointF>
#include <QPainter>

class ALU : QWidget
{
  Q_OBJECT;
  public:
    QPolygon shape;
    QPointF centerPoint;
    QPointF* t;
    float r;
    QPointF * s;
  private:
    QPoint ALabelPoint;
    QPoint BLabelPoint;
    QPoint OutLabelPoint;
    QPoint  APoint;
    QPoint  BPoint;
    QPoint  OutPoint;
  public:
    ALU();
    QRect boundingRect();
    QPoint getAPoint();
    QPoint getBPoint();
    QPoint getOutPoint();
    void draw(QPainter& p);
    void Redraw(QPainter& p);
};
#endif

//ALU drawing code

#include "ALU.h"
#include <QVector>
#include <QPointF>
#include <QPen>
#include <QPolygonF>
#include <QtGlobal>

#include <cmath>
#include <iostream>
#include <sstream>

ALU::ALU()
{


  shape << QPoint(180,440);
  shape << QPoint(130,440);
  shape << QPoint(105,410);
  shape << QPoint(80,440);
  shape << QPoint(30,440);
  shape << QPoint(70,390);
  shape << QPoint(140,390);
  ALabelPoint.setX(55);
  ALabelPoint.setY(440);
  BLabelPoint.setX(145);
  BLabelPoint.setY(440);
  OutLabelPoint.setX(92);
  OutLabelPoint.setY(400);
  centerPoint.setX(105);
  centerPoint.setY(415);
  APoint = ALabelPoint;
  APoint.setY(APoint.y() + 10);
  BPoint = BLabelPoint;
  BPoint.setY(BPoint.y() + 10);
  OutPoint = OutLabelPoint;
  OutPoint.setY(OutPoint.y() - 10);
}
QRect ALU::boundingRect()
{
  return shape.boundingRect();
}
QPoint ALU::getAPoint()
{
  return APoint;
}
QPoint ALU::getBPoint()
{
  return BPoint;
}
QPoint ALU::getOutPoint()
{
  return OutPoint;
}
void ALU::draw(QPainter& p)
{
  Redraw(p);
}
void ALU::Redraw(QPainter& p)
{
  p.save();
  for(int i=0;i<shape.size();i++)
  {
    p.drawLine(shape.point(i),shape.point((i+1)%shape.size()));
  }
  p.drawText(ALabelPoint,"A");
  p.drawText(BLabelPoint,"B");
  p.drawText(OutLabelPoint,"Out");
  //p.rotateAroundPoint(108,centerPoint); I give up
  p.restore();
}

#include "qpainter.h"

#include "RectHelpers.h"

QRect ExpandRectWidth(const QRect& r, int i)
{
  QRect r1(r);
  r1.setWidth(r.width() + i);
  return r1;
}

QRect ExpandRectHeight(const QRect& r, int i)
{
  QRect r1(r);
  r1.setHeight(r.height() + i);
  return r1;
}

QRect ExpandRectWidthHeight(const QRect& r, int w, int h)
{
  QRect r1(r);
  r1.setWidth(r.width() + w);
  r1.setHeight(r.height() + h);
  return r1;
}

QRect MoveRectRightBy(const QRect& r, int i)
{
  QRect r1(r);
  r1.moveRight(r.right() + i);
  return r1;
}

QRect MoveRectLeftBy(const QRect& r, int i)
{
  //cout << "Move Left By, orig x " << r.x() << " i " << i << " width " << r.width() << endl;
  QRect r1(r);
  int w = r1.width();
  r1.setLeft(r.x() - i);
  r1.setWidth(w);
  //r1.moveLeft(r.left() + i);
  //cout << "Move Left By, new x " << r1.x() << " width " << r1.width() << endl;
  return r1;
}

QRect MoveRectUpBy(const QRect& r, int i)
{
  QRect r1(r);
  r1.moveTop(r.top() - i);
  return r1;
}

QRect MoveRectDownBy(const QRect& r, int i)
{
  QRect r1(r);
  r1.moveBottom(r.bottom() + i);
  return r1;
}

void EraseRect(QPainter& p, const QRect& r)
{
  QBrush whiteBrush(Qt::white);
  p.fillRect(r, whiteBrush);
}

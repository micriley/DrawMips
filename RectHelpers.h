#ifndef __RECT_HELPERS__
#define __RECT_HELPERS__

#include <qrect.h>

class QPainter;

// Some simple rectangle manipulation routines
QRect ExpandRectWidth(const QRect& r, int i);

QRect ExpandRectHeight(const QRect& r, int i);

QRect ExpandRectWidthHeight(const QRect& r, int w, int h);

QRect MoveRectRightBy(const QRect& r, int i);

QRect MoveRectLeftBy(const QRect& r, int i);

QRect MoveRectUpBy(const QRect& r, int i);

QRect MoveRectDownBy(const QRect& r, int i);

void EraseRect(QPainter&, const QRect&);

#endif

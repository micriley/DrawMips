#include <iostream>
#include <math.h>

#include <sstream>
#include <set>
#include <vector>
#include <Qt/qpainter.h>

#include "qdisplay.h"
#include "gthread.h"
#include "gpainter.h"

using namespace std;

class BSort;
BSort* bsort;

// Figure out where to put these
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

class BSort 
{ // Bubble sort
public:
  BSort(QDisplay& qd0, int n0);
  void DrawArray(int n, int lth);
  void DrawEmptyArray(int);
  void DrawElement(GPainter& p, int i, const QRect& r);
  void EraseElement(GPainter& p, int i);
  void EraseRect(GPainter& p, const QRect& r);
  void DrawRightArrow(GPainter&, const QRect&);
  void DrawLeftArrow(GPainter&, const QRect&);
  void EraseRightArrow(GPainter&, int);
  void EraseLeftArrow(GPainter&, int);
  void DrawHLineBefore(int);  // Draw horizontal line before element i
  void Sort(int, int);
  void PrintTitle(const char*);
public:
  int*      d;   // Data to sort
  int       n;   // Number of elements
  int       sz;  // Size of element rectangles
  int       start;  // Start and length of the sub-sort
  int       lth;
  int       mergeX;     // X coordinate of merge array
  int       animHalfX;  // Merge animation halfway point
  QDisplay& qd;
  QRect*    rect;  // Bounding rect for each element
  static set<int> breaks;
};

set<int> BSort::breaks;

BSort::BSort(QDisplay& qd0, int n0)
  : n(n0), sz(0), start(0), lth(n0), qd(qd0), rect(0)
{
  d = new int[n];
  for (int i = 0; i < n; ++i)
    {
      d[i] = (int)(drand48() * n);
    }
  rect = new QRect[n];
  qd.StartMSTimer();
}

void BSort::DrawRightArrow(GPainter& p, const QRect& r)
{ // Draw a right facing arrow to the i'th element
  //  QRect r = rect[i];
  QPoint c = r.center();
  p.setPen(Qt::black);
  int x = r.x();
  int y = c.y();
  p.drawLine(x - sz, y, x - 1, y);
  p.drawLine(x, y, x - sz/4, y - sz/4);
  p.drawLine(x, y, x - sz/4, y + sz/4);
}

void BSort::DrawLeftArrow(GPainter& p, const QRect& r)
{ // Draw a left facing arrow to the i'th element
  QPoint c = r.center();
  p.setPen(Qt::black);
  int x = r.x() + sz;
  int y = c.y();
  p.drawLine(x + sz, y, x + 1, y);
  p.drawLine(x, y, x + sz/4, y - sz/4);
  p.drawLine(x, y, x + sz/4, y + sz/4);
}
             
void BSort::EraseRightArrow(GPainter& p, int i)
{
  QRect r = rect[i];
  r = MoveRectLeftBy(r, sz);
  EraseRect(p, r);
}

void BSort::EraseLeftArrow(GPainter& p, int i)
{
  QRect r = rect[i];
  r = MoveRectRightBy(r, sz + 1);
  EraseRect(p, r);
}

void BSort::DrawHLineBefore(int i)
{ // Draw a horizontal line before element i
  int y = rect[i].y();
  int x0 = sz / 2;
  int x1 = qd.width() - sz / 2;
  GPainter p(qd.Pixmap());
  p.drawLine(x0, y, x1, y);
}
  
void BSort::DrawArray(int start, int lth)
{
  int w = qd.Width();
  //int h = qd.Height();
  int n2 = n + 2; // Allow for margins
  sz = w / n2 - 1;
  int x = w / 2  - sz / 2;
  int y = sz + start * sz;
  GPainter p(qd.Pixmap());

  for (int i = start; i < start + lth; ++i)
    {
      set<int>::const_iterator it = breaks.find(i);
      if (it != breaks.end())
        { // Draw horizontal line here (processor break for parallel)
          y += 2;
          int w = qd.width();
          int x0 = w / 2 - w / 8;
          int x1 = w / 2 + w / 8;
          p.drawLine(x0, y, x1, y);
          y += 2;
        }
      rect[i] = QRect(x, y, sz, sz); // Note location of this element
      DrawElement(p, i, rect[i]);
      y += sz + 1;
    }
}
void BSort::DrawEmptyArray(int x)
{
  mergeX = x;
  animHalfX = qd.Width() / 2 + qd.Width() / 8;
  GPainter p(qd.Pixmap());
  for (int i = 0; i < n; ++i)
    {
      QRect r = rect[i];
      r.moveLeft(x);
      p.drawRect(r);
    }
  QRect r = rect[0];
  r.moveLeft(x);
  DrawLeftArrow(p, r);
  p.end();
  qd.Update();
}

void BSort::DrawElement(GPainter& p, int i, const QRect& r)
{
  p.drawRect(r);
  ostringstream oss;
  oss << d[i];
  p.drawText(r, Qt::AlignCenter, QString(oss.str().c_str()));
}

void BSort::EraseElement(GPainter& p, int i)
{
  QRect r = rect[i];
  r = ExpandRectWidthHeight(r, 1, 1);
  EraseRect(p, r);
}

void BSort::EraseRect(GPainter& p, const QRect& r)
{
  QBrush whiteBrush(Qt::white);
  p.fillRect(r, whiteBrush);
}

void BSort::PrintTitle(const char* title)
{
  if (start == 0)
    { // We only need header once
      GPainter p(qd.Pixmap());
      p.drawText(qd.Pixmap()->rect(), Qt::AlignTop | Qt::AlignHCenter,
                 title);
      p.end();
    }
  qd.Update();
}

void BSort::Sort(int s, int l)
{
  start = s;
  lth = l;
  cout << "Entering Sort main loop" << endl;
  for (int i = s + 1; i < s + lth; ++i)
    {
      GPainter p(qd.Pixmap());
      // Draw new left and right arrows
      DrawRightArrow(p, rect[i]);
      //DrawLeftArrow(p, rect[i]);
      p.end();
      qd.Update();
      //qd.WaitUserTimer(1, 1000);
      qd.WaitForMS(1000);
      int j = s + i;
      while (d[j] < d[j-1])
        { // swap
          // Reduce the rate of the save timer, since writing all the
          // png files slows down the animation
          qd.SaveTimerInterval(75);
          // Now animate the swap
          int swap0 = j;
          int swap1 = j - 1;
          //int rarrow = i;
          //int larrow = j;
          QRect rect0 = rect[swap0];
          QRect rect1 = rect[swap1];
          int   moveBy = 2;
          // Animate in three steps, right/left, up/down, left/right
          // First right/left
          for (int k = 0; k < sz; k += moveBy)
            {
              GPainter p(qd.Pixmap());
              QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
              QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
              // tr1 must be moved left by sz pixels
              tr1 = MoveRectLeftBy(tr1, sz);
              EraseRect(p, tr0);
              EraseRect(p, tr1);
              // Move r0 right andn r1 left
              rect0 = MoveRectRightBy(rect0, moveBy);
              rect1 = MoveRectLeftBy(rect1, moveBy);
              // And redraw them
              DrawElement(p, swap0, rect0);
              DrawElement(p, swap1, rect1);
              //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
              //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
              DrawLeftArrow(p, rect0);
              //qd.WaitUserTimer(1, 1000/(24*3));
              qd.WaitForMS(1000/(24*3));
              qd.Update();
            }
          // Now up (right side) and down (left side)
          for (int k = 0; k < sz + 1; k += moveBy)
            {
              GPainter p(qd.Pixmap());
              QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
              QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
              // tr1 must be moved left by sz pixels
              tr1 = MoveRectLeftBy(tr1, sz);
              EraseRect(p, tr0);
              EraseRect(p, tr1);
              // Move r0 right andn r1 left
              rect0 = MoveRectUpBy(rect0, moveBy);
              rect1 = MoveRectDownBy(rect1, moveBy);
              // And redraw them
              DrawElement(p, swap0, rect0);
              DrawElement(p, swap1, rect1);
              //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
              //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
              DrawLeftArrow(p, rect0);
              //qd.WaitUserTimer(1, 1000/(24*3));
              qd.WaitForMS(1000/(24*3));
              qd.Update();
            }
          // Now left (right side) and right (left side)
          for (int k = 0; k < sz; k += moveBy)
            {
              GPainter p(qd.Pixmap());
              QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
              QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
              // tr1 must be moved left by sz pixels
              tr1 = MoveRectLeftBy(tr1, sz);
              EraseRect(p, tr0);
              EraseRect(p, tr1);
              // Move r0 right andn r1 left
              rect0 = MoveRectLeftBy(rect0, moveBy);
              rect1 = MoveRectRightBy(rect1, moveBy);
              // And redraw them
              DrawElement(p, swap0, rect0);
              DrawElement(p, swap1, rect1);
              //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
              //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
              DrawLeftArrow(p, rect0);
              //qd.WaitUserTimer(1, 1000/(24*3));
              qd.WaitForMS(1000/(24*3));
              qd.Update();
            }

          int tmp = d[j];
          d[j] = d[j-1];
          d[j-1] = tmp;
          j--;
          GPainter p1(qd.Pixmap());
          DrawRightArrow(p1, rect[i]);
          DrawLeftArrow(p1, rect[j]);
          // Erase the left arrow from swap
          QRect la1 = rect[j];
          la1 = MoveRectRightBy(la1, sz + 1);
          la1 = ExpandRectWidth(la1, 1);
          EraseRect(p1, la1);
          qd.Update();
          if (j == s) break;
        }
      // Restore full speed save timer
      qd.SaveTimerInterval(1000/25); // 25 fps
      GPainter p1(qd.Pixmap());
      // Erase prior right arrow
      QRect ra1 = rect[i];
      ra1 = MoveRectLeftBy(ra1, sz);
      EraseRect(p1, ra1);
    }
  GPainter p1(qd.Pixmap());
  p1.drawText(qd.Pixmap()->rect(), Qt::AlignBottom | Qt::AlignHCenter,
             "Sorted!");
  p1.end();
  qd.Update();
}

// Parallel version
class BSortP : public BSort 
{
public:
  typedef enum 
    {
      CheckI, CheckJ, AnimateSwapping, Init, None, FindSmallest, AnimateMerge
    } StepType_t;
    
public:
  BSortP(QDisplay&, int n0);
  void Sort(int, int);
  bool Step();
  void DrawArrows();
  bool Done() const;
  void AnimateSwap(GPainter&);
  void InitMerge(GPainter&);
  bool SmallestAvailable();
  int  Smallest();
  void Advance();
  void Merge(); // Only called on zeroth BSortP object
    
  static bool StepAll();
  int i;
  int j;
  bool done;
  StepType_t stepType;
  int  animCount;   // Count for animating moves
  bool animating;   // True if animating a swap
  int  nextStepTime;// Time (ms) of next step
  QRect rect0;      // Rectangles for swap animation
  QRect rect1;
  int   swap0;      // Indices for rectangles being swapped
  int   swap1;
  int   mergeNext;  // Next used index in merged array
  int   mergeJ;     // Left side index of merge animation
  static vector<BSortP*> sorts; // Vector of all BSortP instances
};

vector<BSortP*> BSortP::sorts;

BSortP::BSortP(QDisplay& qd, int n0)
  : BSort(qd, n0), i(0), j(0), done(false), stepType(Init),
    animCount(0), animating(false), nextStepTime(0), swap0(0), swap1(0),
    mergeNext(0), mergeJ(0)
{
  qd.StartMSTimer();
}

#define FAST_STEP (1000/10)
#define SLOW_STEP 1000

void BSortP::Sort(int s, int l) 
{
  start = s;
  lth = l;
  i = s + 1;
  j = s + 1;
  nextStepTime = qd.msTime + FAST_STEP;
}

bool BSortP::Step()
{
  if (done) return false;
  qd.ProcessEvents();
  // See if time to execute next step
  if (nextStepTime > qd.msTime) return true;
  GPainter p(qd.Pixmap());
  switch(stepType)
    {
    case Init:
      stepType = CheckI;
      nextStepTime = qd.msTime + SLOW_STEP + 100 * start;
      DrawRightArrow(p, rect[i]);
      DrawLeftArrow(p, rect[j]);
      break;
    case AnimateSwapping:
      AnimateSwap(p);
      break;
    case CheckI:
      if (d[i] < d[i-1])
        {
          stepType = CheckJ;
          nextStepTime = qd.msTime + FAST_STEP;
        }
      else
        {
          EraseRightArrow(p, i);
          EraseLeftArrow(p, j);
          i++;
          if (i == (start + lth)) done = true;
          if (!Done())
            {
              DrawRightArrow(p, rect[i]);
              j = i;
              DrawLeftArrow(p, rect[j]);
            }
          nextStepTime = qd.msTime + SLOW_STEP;
        }
      break;
    case CheckJ:
      if ((j > start) && (d[j] < d[j-1]))
        {
          stepType = AnimateSwapping;
          rect0 = rect[j];
          rect1 = rect[j-1];
          swap0 = j;
          swap1 = j - 1;
          animCount = 0;
          // Move this to end of animate
        }
      else
        {
          stepType = CheckI;
          nextStepTime = qd.msTime + SLOW_STEP;
        }
      break;
    case FindSmallest:
      {
        int first = true;
        int j = 0;
        int small = 0;
        for (unsigned i = 0; i < sorts.size(); ++i)
          {
            if (sorts[i]->SmallestAvailable())
              {
                if (first)
                  {
                    j = i;
                    small = sorts[i]->Smallest();
                    first = false;
                  }
                else
                  {
                    if (sorts[i]->Smallest() < small)
                      {
                        small = sorts[i]->Smallest();
                        j = i;
                      }
                  }
              }
          }
        // Rect0 and rect1 are source/dest of the merge animation
        rect0 = sorts[j]->rect[sorts[j]->i];
        rect1 = rect[mergeNext];
        rect1.moveLeft(mergeX);
        swap0 = sorts[j]->i;
        swap1 = mergeNext;
        mergeJ = j;
        stepType = AnimateMerge;

      }
      break;
    case AnimateMerge:
      {
        QRect rect2 = ExpandRectWidthHeight(rect0, 1, 1); // To erase
        EraseRect(p, rect2);
        if (rect0.x() < animHalfX)
          { // Move right
            rect0 = MoveRectRightBy(rect0, 1);
            DrawElement(p, swap0, rect0);
            // Redraw original that got overwritten
            sorts[mergeJ]->EraseElement(p, swap0);
            sorts[mergeJ]->DrawElement(p, swap0, sorts[mergeJ]->rect[swap0]);
          }
        else if (rect0.y() != rect1.y())
          { // Move Up/down
            if (rect0.y() < rect1.y())
              { // Move down
                rect0 = MoveRectDownBy(rect0, 1);
                DrawElement(p, swap0, rect0);
              }
            else
              { // Move up
                rect0 = MoveRectUpBy(rect0, 1);
                DrawElement(p, swap0, rect0);
              }
          }
        else
          { // Move right again
            rect0 = MoveRectRightBy(rect0, 1);
            DrawElement(p, swap0, rect0);
            // Below only if all done
            if (rect0.x() == mergeX)
              { // All done
                EraseRightArrow(p, swap0);
                sorts[mergeJ]->Advance();
                if (sorts[mergeJ]->SmallestAvailable())
                  {
                    DrawRightArrow(p, sorts[mergeJ]->rect[sorts[mergeJ]->i]);
                  }
                QRect r1 = rect0;
                r1.moveLeft(mergeX + sz + 1); // To use to earse arrow
                sorts[mergeJ]->EraseRect(p, r1);
                mergeNext++;
                if (mergeNext == n)
                  {
                    done = true;
                  }
                else
                  {
                    QRect r2 = rect[mergeNext];
                    r2.moveLeft(mergeX);
                    DrawLeftArrow(p, r2);
                  }
                nextStepTime = qd.msTime + SLOW_STEP;
                stepType = FindSmallest;
              }
          }
      }
      break;
    case None:
      nextStepTime = qd.msTime + SLOW_STEP;
      break;
    }
  p.end();
  qd.Update();
  return true;
}

void BSortP::DrawArrows()
{
  GPainter p(qd.Pixmap());
  DrawRightArrow(p, rect[i]);
  DrawLeftArrow(p, rect[j]);
  qd.WaitForMS(100);
  qd.Update();
  //qd.WaitUserTimer(1, 1000);
}

bool BSortP::Done() const
{
  return done;
}

void BSortP::AnimateSwap(GPainter& p)
{
  int moveBy = 1;
  switch (animCount) {
  case 0:
    {
      QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
      QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
      // tr1 must be moved left by sz pixels
      tr1 = MoveRectLeftBy(tr1, sz);
      EraseRect(p, tr0);
      EraseRect(p, tr1);
      // Move r0 right and r1 left
      rect0 = MoveRectRightBy(rect0, moveBy);
      rect1 = MoveRectLeftBy(rect1, moveBy);
      // And redraw them
      DrawElement(p, swap0, rect0);
      DrawElement(p, swap1, rect1);
      //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
      //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
      DrawLeftArrow(p, rect0);
      if (rect0.x() == rect[swap0].right()) animCount++;
    }
    break;
  case 1:
    { // Now up (right side) and down (left side)
      QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
      QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
      // tr1 must be moved left by sz pixels
      tr1 = MoveRectLeftBy(tr1, sz);
      EraseRect(p, tr0);
      EraseRect(p, tr1);
      // Move r0 right andn r1 left
      rect0 = MoveRectUpBy(rect0, moveBy);
      rect1 = MoveRectDownBy(rect1, moveBy);
      // And redraw them
      DrawElement(p, swap0, rect0);
      DrawElement(p, swap1, rect1);
      //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
      //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
      DrawLeftArrow(p, rect0);
      if (rect0.y() == rect[swap0-1].y()) animCount++;
    }
    break;
  case 2:
    {
      QRect tr0 = ExpandRectWidthHeight(rect0, sz + 1, 1);
      QRect tr1 = ExpandRectWidthHeight(rect1, sz + 1, 1);
      // tr1 must be moved left by sz pixels
      tr1 = MoveRectLeftBy(tr1, sz);
      EraseRect(p, tr0);
      EraseRect(p, tr1);
      // Move r0 right andn r1 left
      rect0 = MoveRectLeftBy(rect0, moveBy);
      rect1 = MoveRectRightBy(rect1, moveBy);
      // And redraw them
      DrawElement(p, swap0, rect0);
      DrawElement(p, swap1, rect1);
      //cout << "Rect0 x " << rect0.x() << " y " << rect0.y() << endl;
      //cout << "Rect1 x " << rect1.x() << " y " << rect1.y() << endl;
      DrawLeftArrow(p, rect0);
      if (rect0.x() == rect[swap0].x()) animCount++;
    }
    break;
  default:
    { // Done with animation, swap the values and return to CHECKJ
      int tmp = d[j];
      d[j] = d[j-1];
      d[j-1] = tmp;
      //EraseLeftArrow(p, j);
      j--;
      //DrawLeftArrow(p, rect[j]);
      stepType = CheckJ;
    }
    DrawRightArrow(p, rect[i]);
    nextStepTime = qd.msTime + FAST_STEP;
  }
}

void BSortP::InitMerge(GPainter& p)
{ // Initialized the merge activity
  i = start;
  done = false;
  mergeNext = 0;
  nextStepTime = qd.msTime + SLOW_STEP * 3;
  stepType = None;
  DrawRightArrow(p, rect[i]);
}

bool BSortP::SmallestAvailable()
{
  return i < (start + lth);
}

int BSortP::Smallest()
{
  return d[i];
}

void BSortP::Advance()
{
  i++;
}

void BSortP::Merge()
{
  stepType = FindSmallest;
  while(Step()) { }
}

bool BSortP::StepAll()
{
  bool found = false;
  for (unsigned i = 0; i < sorts.size(); ++i)
    {
      BSortP* b = sorts[i];
      if (!b->Done())
        {
          b->Step();
          found = true;
        }
    }
  return found;
}

      
typedef void* (*PThread)(void*);
pthread_mutex_t coutMutex;

// This does not work!
void SortThread(int start, int lth)
{
  ThreadStart();
  pthread_mutex_lock(&coutMutex);
  cout << "Starting thread at " << start << " lth " << lth << endl;
  pthread_mutex_unlock(&coutMutex);
  bsort->Sort(start, lth);
  ThreadEnd();
}

int main(int argc, char** argv)
{
  pthread_mutex_init(&coutMutex, 0);
  int nElements = 20;
  int nThreads = 2;
  string saveName;
  string saveType;
  QApp app(argc, argv);
  QDisplay display(app);
  if (argc > 1) nElements = atol(argv[1]);
  if (argc > 2) nThreads  = atol(argv[2]);
  if (argc > 3) saveName = (argv[3]);
  if (argc > 4) saveType = (argv[4]);
  if (!saveName.empty()) display.SaveName(saveName.c_str(), saveType.c_str());
  display.BlankPixmap(512, 640);
  display.Show();
#define USE_BSORTP
#ifdef  USE_BSORTP
  vector<int> starts;
  vector<int> lengths;
  int perThread = nElements / nThreads;
  int currentStart = 0;
  // Set the starting and length of each thread
  for (int t = 0; t < nThreads; ++t)
    {
      starts.push_back(currentStart);
      lengths.push_back(perThread);
      if (t == (nThreads - 1)) lengths.back() = nElements - currentStart;
      if (currentStart != 0)
        {
          BSortP::breaks.insert(currentStart);
        }
      currentStart += perThread;
    }

  // Now create the sort objects and initialize them
  BSortP* bsortp0;
  for (int t = 0; t < nThreads; ++t)
    {
      BSortP* bsortp;
      if (t == 0)
        {
          bsortp0 = new BSortP(display, nElements); // New one
          bsortp0->DrawArray(0, nElements);
          bsortp0->PrintTitle("Parallel Bubble Sort");
          bsortp = bsortp0;
        }
      else
        {
          bsortp = new BSortP(*bsortp0);  // Copy existing
          bsortp->stepType = BSortP::Init;
        }
      BSortP::sorts.push_back(bsortp);
      bsortp->Sort(starts[t], lengths[t]); // Set initial state
    }
  //bsortp0->DrawHLineBefore(nElements/2); // Moved to draw array
  while(BSortP::StepAll()) { }
  cout << "BSortP done, now merging" << endl;
  bsortp0->DrawEmptyArray(display.Width() / 2 + display.Width() / 4);
  GPainter p(display.Pixmap());
  for (int t = 0; t < nThreads; ++t)
    {
      BSortP::sorts[t]->InitMerge(p);
    }
  display.Update();
  p.end();
  bsortp0->stepType = BSortP::FindSmallest;
  while(bsortp0->Step()) { }
  
  cout << "Merge Complete" << endl;
  app.Run();
  exit(0);
#endif

  bsort = new BSort(display, nElements);

#undef  USE_THREAD
#ifdef  USE_THREAD
  // Threaded version (testing)
  display.UpdateRate(24);
  GPainter::InitMutex();
  int nThread = 1;
  int nextStart = 0;
  int perThread = nElements / nThread;
  InitializeGThreads();
  for (int k = 0; k < nThread; ++k)
    {
      //ThreadInfo_t* ti = new ThreadInfo_t;
      //ti->start = nextStart;
      //if (k < (nThread - 1)) ti->lth = perThread;
      //else ti->lth = nElements - nextStart;
      //pthread_t thr;
      //pthread_create(&thr, 0, (PThread)SortThread, ti);
      //nextStart += ti->lth;
      int n = nextStart;
      int l = perThread;
      if (k == (nThread - 1)) l = nElements - nextStart;
      ThreadCreate(SortThread, n, l);
      nextStart += l;
    }
  WaitAllThreads();
  app.Run();
  exit(1);
#endif  

  //  display.UpdateRate(24);
  bsort->Sort(0, nElements);
  display.WaitUserTimer(1, 5000); // 5 seconds at end of sort for the movie
  display.SaveName(0, 0);
  app.Run();
}

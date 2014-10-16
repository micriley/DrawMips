// Interface to Qt graphics library
// George F. Riley, Georgia Tech, Spring 2004
// ECE3090, Spring 2004

#include <iostream>
#include <fstream>
#include <sstream>
#include <pthread.h>

#include <QPointF>
#include "qdisplay.h"
#include <QTimer>
#include <QPainter>
#include <QEvent>
#include <QVector>
#include <QTransform>
#include <QPolygonF>
#include <QPaintEvent>

#define nil 0

using namespace std;

int QDisplay::windowCount = 0;

#ifdef DEBUG_REMOVE
// QApp methods
QApp::QApp(int argc, char** argv) 
    : QApplication(argc, argv)
{
  // Connect application "lastwindowclosed" signal
  //connect(this, SIGNAL(lastWindowClosed()), SLOT(mainClosed()));
  connect(this, SIGNAL(lastWindowClosed()), SLOT(quit()));
}

void QApp::Run()
{
  exec();
}
#endif

//void QApp::mainClosed()
//{
//  quit();
//}

// Constructors
QDisplay::QDisplay(QApplication& a)
  : ready(true), closed(false), qTimer(nil), saveTimer(nil), msTimer(nil),
    pixmap(nil), updateCallback(nil),
    app(a), updateRate(0), colorTable(nil), white(nil), whitew(0), whiteh(0),
    saveName(nil), saveType(nil), saveIndex(0), msTime(0), paused(false),
    latexSaveFormat(false)
{
  // Initialize the mutex
  pthread_mutex_init(&updateMutex, 0);
  // Create and connect the timer
  qTimer = new QTimer();
  connect(qTimer, SIGNAL(timeout()), SLOT(timerDone()));  
  saveTimer = new QTimer();
  connect(saveTimer, SIGNAL(timeout()), SLOT(SaveTimerDone()));
  userTimers = new QTimer[2];
  connect(&userTimers[0], SIGNAL(timeout()), SLOT(UserTimer0Done()));
  connect(&userTimers[1], SIGNAL(timeout()), SLOT(UserTimer1Done()));
  userReady = new bool[2];
}

QDisplay::~QDisplay()
{ // Destructor
  if (qTimer) qTimer->stop();
  if (saveTimer) saveTimer->stop();
  delete qTimer;
  delete saveTimer;
  delete colorTable;
  delete white;
}

// Member functions
bool QDisplay::Load(const char*)
{ // Load an image from a file
#ifdef FIX_LATER
  image = new QImage();
  if (!image->load(fn))
    { // load failed
      delete image;
      image = nil;
      return false;
    }
  resize(image->width(), image->height()); // Size the main window to image
  Update();
  if (windowCount++==0)
    {
      //app.setMainWidget(this);
    }
#endif
  return true;
}

bool QDisplay::Save(const  char*)
{
#ifdef FIX_LATER
  if (!image) return false; // no image to save
  return image->save(fn, "PNG");
#ifdef TRY_PNM
  if (Depth() == 8)
    return image->save(fn, "PNM");
  else
    return image->save(fn, "PNG");
#endif
#ifdef OLD_WAY
  ofstream fs(fn);
  if (!fs) return false;
  fs << "P5" << endl;
  fs << "# CREATOR Michael K. RILEY" << endl;
  fs << "# GENERATOR: QDisplay::Save" << endl;
  fs << image->width() << " " << image->height() << endl;
  fs << image->numColors() << endl;
  unsigned char* b = image->bits();
  for (int y = 0; y < image->height(); ++y)
    {
      for (int x = 0; x < image->width(); ++x)
        fs << b[y*image->width() + x];
    }
  fs << endl;
  fs.close();
  return true;
#endif
#endif
  return true;
}

#ifdef FIX_LATER
void QDisplay::BlankImage(int w, int h, int d)
{  // Create a blank (all white) image
  int colorCount;
  
  if (d == 8)
    {
      if (!colorTable)
        {
          colorTable = new QRgb[256];
          for (int i = 0; i < 256; ++i)
            {
              colorTable[i] = QColor(i, i, i).rgb();
            }
        }
      if (whitew != w || whiteh != h || whited != d)
        { // Existing white buffer wrong size
          delete [] white;
          white = new unsigned char[w * h];
          memset(white, 0xff, w * h);
          whitew = w;
          whiteh = h;
          whited = d;
        }
      colorCount = 256;
    }
  else
    {
      colorCount = 0; // 32 bit color
      if (whitew != w || whiteh != h || whited != d)
        { // Existing white buffer wrong size
          delete [] white;
          white = (unsigned char*)new QRgb[w * h];
          for (int i = 0; i < w * h; ++i)
            ((QRgb*)white)[i] = QColor(255,255,255).rgb();
        }
      
    }
  whitew = w;
  whiteh = h;
  whited = d;
  
  if (image)
    { // Delete any existing
      delete image;
      image = nil;
    }
  
  // Create an image, all white
  image = new QImage(white, w, h, d,
                     colorTable, colorCount, QImage::IgnoreEndian);
  resize(image->width(), image->height()); // Size the main window to image
  //Update(); // And draw on screen
  if (windowCount++==0)
    {
      app.setMainWidget(this);
    }
}
#endif

void QDisplay::BlankPixmap(int w, int h)
{ // Create a blank pixmap
  delete pixmap;
  pixmap = new QPixmap(w, h);
  pixmap->fill();
  resize(w, h); // Size the main window to image
}

int  QDisplay::Depth()
{ // Return image depth (bits per pixel), either 1, 8, or 32.
  // Note, the 32 bit images are actually 24 bits, with 8 bits of filler
  // for word alignment
  if (!pixmap) return 0; // No image loaded
  int d = pixmap->depth();
  if (d == 24) d = 32;
  return d;             // Bits per pixel
}

int  QDisplay::Width()
{ // Return image width (pixels)
  if (!pixmap) return 0;
  return pixmap->width();
}

int  QDisplay::Height()
{ // Return pixmap height (pixels)
  if (!pixmap) return 0;
  return pixmap->height();
}
  
unsigned char* QDisplay::ImageData()
{ // Get a pointer to the image data
#ifdef FIX_LATER
  if(!image) return nil; // No image loaded
  //cout << "Image data has " << image->numBytes() << " bytes " << endl;
  return image->bits();
#endif
  return nil;
}

void  QDisplay::Show(bool s)
{
  if (s) show();
  else   hide();
}

void  QDisplay::Update()
{
  if (closed) return; // No more updates after window closed
  pthread_mutex_lock(&updateMutex);
  if (updateCallback)updateCallback();
  
  while(!ready && !closed)
    {
      pthread_yield();
      app.processEvents(); // Wait for 25fps elapsed 
    }
  if (updateRate)
    { // Non-zero update rate specified
      ready = false;
      qTimer->start(1000/updateRate);        // 40ms is 25 frames per secnod
    }
  update();                                  // Force re-paint event later
  //repaint();                                  // Force re-paint event now
  //while(app.hasPendingEvents()) app.processEvents();
  app.processEvents();
  pthread_mutex_unlock(&updateMutex);
}

void  QDisplay::Update(int x, int y, int w, int h, bool noPaint)
{
  if (closed) return; // No more updates after window closed
  pthread_mutex_lock(&updateMutex);
  if (!noPaint)
    {
      if (updateCallback)updateCallback();
      while(!ready && !closed)
        {
          app.processEvents(); // Wait for 25fps elapsed
          pthread_yield();
        }
      if (updateRate)
        { // Non-zero update rate specified
          ready = false;
          qTimer->start(1000/updateRate);     // Start timer for next update
        }
    }
  
  
  // Update the pixel
  if (pixmap)
    {
      //repaint(x, y, 1, 1);
      update(x, y, w, h);
    }
  if (!noPaint) app.processEvents();
  pthread_mutex_unlock(&updateMutex);
}

void QDisplay::Update(const QRect& r, bool noPaint)
{
  Update(r.x(), r.y(), r.width(), r.height(), noPaint);
}

void QDisplay::SetUpdateCallback(UpdateCallback_t upd)
{
  updateCallback = upd;
}


bool QDisplay::IsClosed()
{
  return closed;
}

void QDisplay::ProcessEvents()
{
  app.processEvents();
}

void QDisplay::UpdateRate(int r)
{
  updateRate = r;
}

void QDisplay::SaveName(const char* fn, const char* ftype)
{
  saveName = fn;
  saveType = ftype;
  if (!fn) saveTimer->stop();
  else     saveTimer->start(100); // 10 fps
  //else     saveTimer->start(1000/25); // 25 fps
}

void QDisplay::SaveTimerInterval(int ms)
{
  if (saveTimer && saveName)
    {
      saveTimer->stop();
      saveTimer->start(ms);
    }
}

// Inherited from QMainWindow
void QDisplay::paintEvent(QPaintEvent* pe)
{
  if (pixmap)
    {
      QRect r = pe->rect();
      //bitBlt(this, r.x(), r.y(), pixmap, r.x(), r.y(), r.width(), r.height());
      QPainter p(this);
      p.drawPixmap(r.x(), r.y(), *pixmap, r.x(), r.y(), r.width(), r.height());
    }
}

void QDisplay::mousePressEvent(QMouseEvent*)
{
  //cout << "Mouse pressed, x " << e->x()
  //     << " y " << e->y() << endl;
  paused = !paused; // Use mouse for pausing
}

void QDisplay::DrawLine(QPoint A,QPoint B,QColor color)
{
  DrawLine(A.x(),A.y(),B.x(),B.y(),color);
}
// Add painter interfaces
void QDisplay::DrawLine(int x0, int y0, int x1, int y1, QColor color)
{
  QPainter p(pixmap);
  p.setPen(color);
  p.drawLine(x0, y0, x1, y1);
}

void QDisplay::rotateAroundPoint(int rotate, QPoint centerPoint)
{
  QPainter p(pixmap);
  p.translate(-centerPoint.x(),-centerPoint.y());
  p.rotate(rotate);
  p.translate(centerPoint.x(),centerPoint.y());
}

void QDisplay::DrawSquare(int x0, int y0, int wh, QColor color, bool update)
{
  int wh2 = wh/2;
  DrawLine(x0 - wh2, y0 - wh2, x0 + wh2, y0 - wh2, color);
  DrawLine(x0 + wh2, y0 - wh2, x0 + wh2, y0 + wh2, color);
  DrawLine(x0 + wh2, y0 + wh2, x0 - wh2, y0 + wh2, color);
  DrawLine(x0 - wh2, y0 + wh2, x0 - wh2, y0 - wh2, color);
  if (update)Update(x0 - wh2, y0 - wh2, wh, wh);
}
  
QPixmap* QDisplay::Pixmap()
{
  return pixmap;
}

void QDisplay::WaitUserTimer(int i, int ms)
{ // Wait for i'th timer, ms milliseconds
  return;
  
  userReady[i] = false;
  userTimers[i].start(ms);
  while(!userReady[i])
    {
      app.processEvents();
      pthread_yield();
    }
}

void QDisplay::StartMSTimer()
{
  if (!msTimer)
    {
      msTimer = new QTimer();
      connect(msTimer, SIGNAL(timeout()), SLOT(MSTimerDone()));
      msTimer->start(10);
    }
}

void QDisplay::WaitForMS(int endMS)
{
  int endTime = msTime + endMS;
  StartMSTimer();
  while (msTime < endTime || paused)
    {
      //cout << "Waiting ms, now " << msTime << " endtime " << endTime << endl;
      app.processEvents();
      pthread_yield();
    }
}

void QDisplay::Clear(QColor color, bool update)
{ // Clear the pixmap
  pixmap->fill(color);
  if (update) Update();
}


// Slots
void QDisplay::timerDone()
{
  ready = true; // Enough time elapsed for another frame update
}

void QDisplay::UserTimer0Done()
{
  userReady[0] = true; // Enough time elapsed for another frame update
}

void QDisplay::UserTimer1Done()
{
  userReady[1] = true; // Enough time elapsed for another frame update
}

void QDisplay::MSTimerDone()
{
  msTime += 10;
  msTimer->start(10);
}

void QDisplay::SaveTimerDone()
{
  string fn(saveName);
  string ftype(".png");
  if (saveType) ftype = string(".") + string(saveType);
  ostringstream oss;
  if (!latexSaveFormat)
    {
      oss.width(5);
      oss.fill('0');
    }
  else
    {
      oss << "-";
    }
  oss << saveIndex++;
  string sn = fn + oss.str() + ftype;
  //cout << "Hello from SaveTimerDone, name " << sn << endl;
  if (saveType)
    pixmap->save(sn.c_str(), saveType);
  else
    pixmap->save(sn.c_str());
}

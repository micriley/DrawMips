// Interface to Qt graphical display.
// George F. Riley, Georgia Tech, Spring 2004
// ECE3090

#include <Qt/qapplication.h>
#include <Qt/qmainwindow.h>
#include <pthread.h>

class QTimer;
class QImage;

typedef void (*UpdateCallback_t)(void);

#ifdef DEBUG_REMOVE
class QApp : public QApplication 
{
  //Q_OBJECT
public :
  QApp(int argc, char** argv);
public:
  void  Run();                // Process events until all windows closed
  //public slots:
  //void mainClosed();
};
#endif

class QDisplay : public QMainWindow 
{
Q_OBJECT
public:
  QDisplay(QApplication&);    // Requires a QApplication object
  virtual ~QDisplay();        // Destructor
  bool  Load(const char*);    // Load image from a file
  bool  Save(const char*);    // Save image to a file
  //void  BlankImage(int, int, int); // Create a blank grayscale image, w h d
  void  BlankPixmap(int, int);     // Blank pixmap, default depth
  int   Depth();              // Get image depth
  int   Width();              // Get image width
  int   Height();             // Get image height
  unsigned char* ImageData(); // Get image data
  void  Show(bool s = true);  // Set window visibility
  void  Update();             // Update the entire screen image
  void  SetUpdateCallback(UpdateCallback_t);
  // Update a rectangular area
  void  Update(int x,int y, int w = 1, int h = 1, bool noPaint = false);
  void  Update(const QRect&, bool noPaint = false);
  void  ProcessEvents();      // Process any outstanding qt events
  bool  IsClosed();           // True if main window is closed 
  void  UpdateRate(int);      // Set update rate (frames/sec) 0 = infinite
  void  SaveName(const char* fn, const char* ftype);  // Base name video saving
  void  SaveTimerInterval(int);    // Set the interval for the save timer
  // Inherited from QMainWindow
  void paintEvent(QPaintEvent*);
  void mousePressEvent(QMouseEvent*);
  void DrawLine(QPoint A,QPoint B, QColor = Qt::black);
  void DrawLine(int x0, int y0, int x1, int y1, QColor = Qt::black);
  // Square upper left at x0/y0, side wh    
  void DrawSquare(int x0, int y0, int wh, QColor = Qt::black, bool update = false); 
  void rotateAroundPoint(int rotate, QPoint centerPoint);
  QPixmap* Pixmap();
  void Clear(QColor = Qt::white,bool update = false);
  void WaitUserTimer(int, int); // timer number, milliseconds
  void StartMSTimer();          // Start the ms timer
  void WaitForMS(int);          // Wait for specified number of milliseconds
  // Slots
public slots:
  void timerDone();
  void SaveTimerDone();
  void UserTimer0Done();
  void UserTimer1Done();
  void MSTimerDone();
private:
  bool           ready;        // If ready to display
  bool*          userReady;    // Array of readys for user timers
  bool           closed;       // If main window closed
  QTimer*        qTimer;       // Ready timer, to enforce 25fps update rate
  QTimer*        saveTimer;    // Used to write pmn files for video encoding
  QTimer*        userTimers;   // Array of two user timers
  QTimer*        msTimer;      // Millisecond timer
  //QImage*        image;        // Current image
  QPixmap*       pixmap;       // Current pixmap
  UpdateCallback_t updateCallback;  // Function to callback before update
  QApplication&  app;          // Application object
  int            updateRate;   // Update rate (frames per sec)
  QRgb*          colorTable;   // Grayscale color table
  unsigned char* white;        // All white for creating blank images
  int            whitew;       // Width of white buffer
  int            whiteh;       // Height of white buffer
  int            whited;       // Depth of white buffer
  const char*    saveName;     // Base name of image save files
  const char*    saveType;     // Type for save files
  int            saveIndex;    // Number for save file 
public:
  int            msTime;       // Milliseconds since timer started
  bool           paused;       // True if animation paused
  bool           latexSaveFormat;
private:
  static int     windowCount;  // Number of open windows
  pthread_mutex_t updateMutex; // Locks pixmap updates
};

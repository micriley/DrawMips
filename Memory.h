#ifndef __MEMORY__
#define __MEMORY__

#include <vector>
#include <map>
#include <string>

#include <qrect.h>

#define VIEW_SIZE 99999

class QPainter;
class QPoint;

typedef enum {
  CT_INT,
  CT_UINT,
  CT_DOUBLE,
  CT_STRING,
  CT_CHAR } Contents_t;

class Contents {
public:
  Contents();
  void SetInt(int i0,bool isSigned);
  void SetDouble(double d0);
  void SetChar(char c0);
  void SetString(std::string st0);
  int GetInt();
  std::string GetString() const; // Get contents, string format
  Contents operator=(Contents c);
public:
  Contents_t  cType;
  int          i;
  unsigned int ui;
  std::string  st;
  double       d;
  char         c;
};

class MemoryLocation {
 public:
  // Setting functions
  void SetContentsGeneric(const std::string&);  // Figure out what type
  void SetContentsInt(int);
  void SetContentsString(const std::string&);
  void SetContentsDouble(double);
  void SetContentsChar(char);
  // Label (variable name)
  void SetLabel(const std::string&);

  // Getting functions
  Contents_t  GetContentsType() const;
  int         GetContentsInt() const;
  std::string GetContentsString() const;
  double      GetContentsDouble() const;
  char        GetContentsChar() const;
 public:
  QRect       addressRect;
  QRect       contentsRect;
  QRect       labelRect;
  std::string address;
  Contents    contents; // int, double, char, string
  std::string label;
  unsigned sourceLine;  //Actual sourceLine for the memory location
  unsigned index;       //index within a Memory Object
  unsigned memoryIndex; //The label number that the memory actually references
};

typedef std::vector<MemoryLocation> MemVec_t;
typedef std::vector<std::string> StringVec_t;
typedef std::map<std::string,MemoryLocation> MemMap_t;

typedef enum {
  MEM_DATA,
  MEM_INST,
  MEM_REG,
} MemType_t;

class Memory
{
  public:
  Memory(MemType_t = MEM_DATA,int fA = 0, int aA = 1,int sML = -1);

  void Load(const QPoint&, const QRect&,
            bool useAddr);
  void LoadEmpty(unsigned n, const QPoint&, const QRect&, bool useAddr);
  void Draw(QPainter& p, QPoint& where, QRect& size,
            bool useAddr,
            const std::string& t1 = std::string(),    // Title1
            const std::string& t2 = std::string());   // Title2
  void Redraw(QPainter& p);
  void AddEmptyMemBlock(unsigned& lineNum);
  void AddMemBlock(MemoryLocation& ml, unsigned& lineNum);
  Contents GetContents(unsigned i);
  MemoryLocation* GetLocation(unsigned i);
  MemoryLocation* GetFront();
  MemoryLocation* GetBack();
  void SetContents(const Contents& c, unsigned i);
  void SetContentsInt(int v, unsigned i);
  void SetContentsString(const std::string& v, unsigned i);
  void SetContentsDouble(double v, unsigned i);
  void SetContentsChar(char v, unsigned i);
  std::string GetContentsString(unsigned i);
  char GetContentsChar();
  double GetContentsDouble();
  int GetContentsInt(unsigned i);
  QRect GetBoundingRect(unsigned i);
  QPoint GetTopCenter(unsigned i);
  QPoint GetBottomCenter(unsigned i);
  QPoint GetLeftCenter(unsigned i);
  QPoint GetRightCenter(unsigned i);
  QPoint GetPointOnMemLoc(unsigned i,float x,float y);
  int FindAddressTag(const std::string&);
  void ParseLine(const std::string&, StringVec_t&);
  unsigned* updateRangeToShow();
  QPoint GetLockedLeftCenter(unsigned i);
	void exitWithOutput(const std::string reason);
private:
  void storeWindowRect(unsigned n);
public:
  QPoint where;  // Upper left
  QRect  size;   // Size of each individual memory location
  std::string title1;
  std::string title2;
  QRect  title1Rect;
  QRect  title2Rect;
  static MemMap_t    labelMap;
  MemVec_t    memory;      // The actual memory
private:
  MemVec_t    windowRects;
  bool        useAddress;  // True if need to use address labels
  MemType_t   memoryType;
public:
  unsigned    firstAddress;   // Address offset for this block
  unsigned    addressAdder;   // Increment for sequential addresses (always 4 for MIPS)
  unsigned    viewSize;
  int    selectedMemLocation; //Currently selected Memory address. -1 means we're turned off
  unsigned*   window; //Range of values to show
};

#endif

// Draw a memory array
// Michael K. Riley, Georgia Tech, Fall 2014

#include <sstream>
#include <iostream>
#include <fstream>

#include <string>
#include <algorithm>
#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>

#include "Memory.h"
#include "RectHelpers.h"

using namespace std;

// Contents methods
Contents::Contents() : cType(CT_STRING), i(0), ui(0), st(), d(0), c(0)
{
}

void Contents::SetInt(int i0,bool isSigned)
{
  if(isSigned){
    cType = CT_INT;
    i = i0;
  }
  else{
    cType = CT_UINT;
    ui = i0;
  }
}

void Contents::SetDouble(double d0)
{
  d = d0;
  cType = CT_DOUBLE;
}

void Contents::SetChar(char c0)
{
  c = c0;
  cType = CT_CHAR;
}

void Contents::SetString(std::string st0)
{
  st = st0;
  cType = CT_STRING;
}

string Contents::GetString() const
{
  ostringstream oss;
  switch(cType)
    {
    case CT_INT :
      oss << i;
      break;
    case CT_UINT:
      oss << ui;
      break;
    case CT_STRING:
      oss << st;
      break;
    case CT_DOUBLE:
      oss << d;
      break;
    case CT_CHAR:
      oss << c;
      break;
    }
  return oss.str();
}

int Contents::GetInt()
{
  int value = 0;
  switch(cType) {
  case CT_INT:
    return i;
  case CT_UINT:
    return (int)ui;
  case CT_DOUBLE:
    return (int)d;
  case CT_STRING:
    return (int)st[0];
  case CT_CHAR:
    return (int)c;
  }
  return value;
}

Contents Contents::operator=(Contents rhs)
{
  i = 0;
  ui = 0;
  st= "";
  d = 0;
  c = 0;
  cType = rhs.cType;
  switch(cType) {
  case CT_INT:
    i = rhs.i;
    break;
  case CT_UINT:
    ui = rhs.ui;
    break;
  case CT_DOUBLE:
    d = rhs.d;
    break;
  case CT_STRING:
    st = rhs.st;
    break;
  case CT_CHAR:
    c = rhs.c;
    break;
  }
  return *this;
}

// Meminfo methods
void MemoryLocation::SetContentsGeneric(const string& st0)
{
  bool hasCharacter = false;
  bool hasSingleDecimal = false;
  bool binary = true;
  for (unsigned i = 0; i < st0.length(); ++i)
    {
      char ch = st0[i];
      if (!isdigit(ch))
        {
          if (ch == '-' && i == 0) continue; // allow leading minus
          if (ch == '.' && !hasSingleDecimal)
            {
              hasSingleDecimal = true;
              continue;
            }
          hasCharacter = true; // Otherwise has alphabetic char (string)
          if (ch != '0' && ch != '1') binary = false;
        }
    }
  // If less-eq 8 bits, just assume integer
  if (st0.length() <= 8) binary = false;
  if (hasCharacter || binary)
    {
      if (st0.length() == 1)
        SetContentsChar(st0[0]);
      else
        SetContentsString(st0);
    }
  else if (hasSingleDecimal) SetContentsDouble(atof(st0.c_str()));
  else SetContentsInt(atol(st0.c_str()));
}

void MemoryLocation::SetContentsInt(int i0)
{
  contents.i = i0;
  contents.cType = CT_INT;
}

void MemoryLocation::SetContentsString(const string& st0)
{
  contents.st = st0;
  contents.cType = CT_STRING;
}

void MemoryLocation::SetContentsDouble(double d0)
{
  contents.d = d0;
  contents.cType = CT_DOUBLE;
}

void MemoryLocation::SetContentsChar(char ch0)
{
  contents.c = ch0;
  contents.cType = CT_CHAR;
}


// Label setting
void MemoryLocation::SetLabel(const string& st)
{
  label = st; // Rectangle is set later in Draw
}

Contents_t MemoryLocation::GetContentsType() const
{
  return contents.cType;
}

int MemoryLocation::GetContentsInt() const
{
  return contents.i;
}

string MemoryLocation::GetContentsString() const
{ // This one is a bit different.  Converts each of the underlyign
  // types to string and returns it;
  return contents.GetString();
}

double MemoryLocation::GetContentsDouble() const
{
  return contents.d;
}

char MemoryLocation::GetContentsChar() const
{
  return contents.c;
}

MemMap_t Memory::labelMap;

Memory::Memory(MemType_t mt, int fA, int aA,int sML)
  : useAddress(true), memoryType(mt), firstAddress(fA), addressAdder(aA),  selectedMemLocation(sML)
{ //Not much to do here, most work done by Load and Draw
  window = (unsigned int*)malloc(2);
}

void Memory::LoadEmpty(unsigned n, const QPoint& wh, const QRect& sz, bool useAddr)
{
  useAddress = useAddr;
  where = wh;
  size = sz;
  for(unsigned i = 0; i < n; ++i)
  {
    if(i == memory.size())
      memory.push_back(MemoryLocation());
  }
}

void Memory::Load(const QPoint& wh, const QRect& sz, bool useAddr)
{
  useAddress = useAddr;
  where = wh;
  size = sz;
}

void Memory::Draw(QPainter& p, QPoint& where, QRect& size,
                  bool useAddr, const string& t1, const string& t2)
{
  int widest = size.width();
  for (unsigned i = 0; i < memory.size(); ++i)
  {
    string contents = memory[i].GetContentsString();
    QRect r = p.boundingRect(size, Qt::AlignCenter, contents.c_str());
    if (r.width() > widest) widest = r.width();
  }
  size.setWidth(widest);
  useAddress = useAddr;
  title1 = t1;
  title2 = t2;
  //cout << "t1 " << t1 << " t2 " << t2 << endl;
  // Start by finding widest address tag and contents, so we can move the actual
  // memory array right by that amount
  int widestAddress = 0;
  int widestContents = 0;
  unsigned extraRect = memoryType == MEM_REG ? 2 : 0;
  for(unsigned i = 0; i < extraRect; ++i)
  {
    memory.push_back(MemoryLocation());
  }
  for (unsigned i = 0; i < memory.size(); ++i)
  {
    ostringstream oss;
    oss << (i*addressAdder + firstAddress);
    QRect br = p.boundingRect(size, Qt::AlignCenter, oss.str().c_str());
    if (br.width() > widestAddress) widestAddress = br.width();
    MemoryLocation& ml = memory[i];
    string st = ml.GetContentsString();
    br = p.boundingRect(size, Qt::AlignCenter, st.c_str());
    if (br.width() > widestContents) widestContents = br.width();
    br = p.boundingRect(size, Qt::AlignCenter, ml.label.c_str());
    if (br.width() > widestAddress) widestAddress = br.width();
  }

  QRect r = size;
  r.moveTo(where);

  QRect a = r; // Rectangle for addresses
  if (useAddress) r = MoveRectRightBy(r, widestAddress + 1);
  a.setWidth(widestAddress);
  // See if contents exceeds width of the outer box
/*  int fudge = 48;
  if ((r.x() + widestContents) > (768 - fudge))
  {
    int amount = where.x() + widestContents - 768 + fudge;
    where.setX(where.x() - amount);
    r = MoveRectLeftBy(r, amount);
    a = MoveRectLeftBy(a, amount);
  }
*/  
  // Draw the titles
  if (!title1.empty())
  {
    QRect t1 = p.boundingRect(r, Qt::AlignCenter, title1.c_str());
    p.drawText(r, Qt::AlignCenter, title1.c_str());
    title1Rect = r;
    // Adjust rect. locations
    r = MoveRectDownBy(r, t1.height());
    a = MoveRectDownBy(a, t1.height());
  }
  if (!title2.empty())
  {
    QRect t2 = p.boundingRect(r, Qt::AlignLeft, title2.c_str());
//		t2 = MoveRectLeftBy(t2, r.width()/2 + 10);
    p.drawText(t2, Qt::AlignCenter, title2.c_str());
		r = MoveRectRightBy(r, 18);
		r.setWidth(size.width() - 14);
    title2Rect = t2;
  }
  // Draw the rectangles
  unsigned thisAddress = 0; // This still needs some work
  for (unsigned i = 0; i < memory.size(); ++i)
  {
    ostringstream oss;
    oss << thisAddress + firstAddress;
    if (useAddress)
    {
      //cout << "Using address " << oss.str() << endl;
      string addr = oss.str();
      p.drawText(a, Qt::AlignVCenter | Qt::AlignRight, addr.c_str());
    }
    p.drawRect(r);
    MemoryLocation& ml = memory[i];
    ml.contentsRect = r;
    ml.address = oss.str();
    if (useAddress) ml.addressRect = a;
    r = MoveRectDownBy(r, size.height() + 1);
    a = MoveRectDownBy(a, size.height() + 1);
    thisAddress += addressAdder;
    // Now show the contents and labels
    string st = ml.GetContentsString();
    if (st.empty()) st = string("?");
    p.drawText(ml.contentsRect, Qt::AlignCenter, st.c_str());
    if (!ml.label.empty())
    { // Draw the label
      QRect r1 = ml.contentsRect;
      r1 = MoveRectRightBy(r1, r1.width() + 8);
      r1 = p.boundingRect(r1, Qt::AlignVCenter | Qt::AlignLeft,ml.label.c_str());
      ml.labelRect = r1;
      p.drawText(ml.labelRect, Qt::AlignVCenter | Qt::AlignLeft,ml.label.c_str());
    }
  }
  int length = memory.size();
  storeWindowRect(std::min(VIEW_SIZE,length));
  window = updateRangeToShow();
}

void Memory::Redraw(QPainter& p)
{
  if (!title1.empty())p.drawText(title1Rect, Qt::AlignCenter, title1.c_str());
  if (!title2.empty())p.drawText(title2Rect, Qt::AlignCenter, title2.c_str());
  if(memoryType == MEM_REG)
  {
    memory[memory.size()-2].address = "HI";
    memory[memory.size()-1].address = "LO";
  }
  // Redraw each memory location
  // the n position to actually draw at
  unsigned n = 0;
  for (unsigned i = window[0]; i <= window[1]; ++i)
  {
    p.setPen(Qt::black);
    if(i == (unsigned)selectedMemLocation)
    {
      p.setPen(Qt::darkGreen);
    }
    MemoryLocation mi = memory[i];
    if(i != n)
    {
      mi.addressRect  = windowRects[n].addressRect;
      mi.contentsRect = windowRects[n].contentsRect;
      mi.labelRect    = windowRects[n].labelRect;
    }
    if (useAddress)
    {
      string addr = mi.address;
      //If we have a label, use that instead of the address number
      if(!mi.label.empty()){
        addr = mi.label;
      }
      //cout << "Using address " << mi.address <<  endl;
      p.drawText(mi.addressRect, Qt::AlignVCenter | Qt::AlignRight, addr.c_str());
    }
    p.drawRect(mi.contentsRect);
    string st = mi.GetContentsString();
    if (st.empty()) st = string("?");
    p.drawText(mi.contentsRect, Qt::AlignCenter, st.c_str());
    n++;
  }
  p.setPen(Qt::black);
}

void Memory::AddEmptyMemBlock(unsigned& lineNum)
{
  memory.push_back(MemoryLocation());
  memory.back().sourceLine = lineNum;
  memory.back().index = memory.size() - 1;
}

void Memory::AddMemBlock(MemoryLocation& ml, unsigned& lineNum)
{
  memory.push_back(ml);
  memory.back().sourceLine = lineNum;
  memory.back().index = memory.size() - 1;
}

Contents Memory::GetContents(unsigned i)
{
  return GetLocation(i)->contents;
}

MemoryLocation* Memory::GetLocation(unsigned i)
{
  return &memory[(i - firstAddress)/addressAdder];
}

MemoryLocation* Memory::GetFront()
{
  return &memory[0];
}

MemoryLocation* Memory::GetBack()
{
  return &memory[memory.size() - 1];
}

string Memory::GetContentsString(unsigned i)
{
  Contents contents = GetContents(i);
  ostringstream oss;
  switch(contents.cType) {
  case CT_INT:
    oss << contents.i;
    break;
  case CT_UINT:
    oss << contents.ui;
    break;
  case CT_DOUBLE:
    oss << contents.d;
    break;
  case CT_STRING:
    oss << contents.st;
    break;
  case CT_CHAR:
    oss << contents.c;
    break;
  }
  return oss.str();
}

int Memory::GetContentsInt(unsigned i)
{
  return GetContents(i).GetInt();
}

QRect Memory::GetBoundingRect(unsigned i)
{
  return GetLocation(i)->contentsRect;
}

QPoint Memory::GetTopCenter(unsigned i)
{
  QRect r = GetBoundingRect(i);
  QPoint p = r.topLeft();
  p.setX(p.x() + r.width() / 2);
  return p;
}

QPoint Memory::GetBottomCenter(unsigned i)
{
  QRect r = GetBoundingRect(i);
  QPoint p = r.bottomLeft();
  p.setX(p.x() + r.width() / 2);
  return p;
}

QPoint Memory::GetLeftCenter(unsigned i)
{
  QRect r = GetBoundingRect(i);
  QPoint p = r.topLeft();
  p.setY(p.y() + r.height() / 2);
  return p;
}

QPoint Memory::GetLockedLeftCenter(unsigned i)
{
  unsigned actualI = i - (window[0]*addressAdder);
  if(actualI < firstAddress)
    actualI = firstAddress;
  return GetLeftCenter(actualI);
}

QPoint Memory::GetRightCenter(unsigned i)
{
  QRect r = GetBoundingRect(i);
  QPoint p = r.topRight();
  p.setY(p.y() + r.height() / 2);
  return p;
}
QPoint Memory::GetPointOnMemLoc(unsigned i,float x,float y)
{
  QRect r = GetBoundingRect(i);
  QPoint p = r.topLeft();
  p.setX(p.x() + r.width() * x);
  p.setY(p.y() + r.height() * y);
  return p;
}

int Memory::FindAddressTag(const string& tag)
{
  for (unsigned i = 0; i < memory.size(); ++i)
    {
      if (memory[i].label == tag)
      {
        return i * addressAdder + firstAddress; // Found it
      }
    }
	return -1;
}

void Memory::SetContents(const Contents& v, unsigned i)
{
  MemoryLocation* ml = GetLocation(i);
  ml->contents = v;
}

void Memory::SetContentsInt(int v, unsigned i)
{
  MemoryLocation* ml = GetLocation(i);
  ml->SetContentsInt(v);
}

void Memory::SetContentsString(const string& v, unsigned i)
{
  MemoryLocation* ml = GetLocation(i);
  ml->SetContentsString(v);
}

unsigned* Memory::updateRangeToShow()
{
  int length = memory.size();
  window[0] = selectedMemLocation == -1 ? 0 : std::max(0,selectedMemLocation-VIEW_SIZE/2);
  window[1] = selectedMemLocation == -1 ? (length-1) : std::min(length-1,selectedMemLocation+VIEW_SIZE/2);
  return window;
}

void Memory::storeWindowRect(unsigned n)
{
  windowRects.clear();
  for(unsigned i = 0;i < n;i++)
  {
    windowRects.push_back(memory[i]);
  }
}

// Static methods
//Makes a string vector of different tokens. We assume they're delinated by ' '
void Memory::ParseLine(const string& st, StringVec_t& tokens)
{ // First look for '#' for comments. Cut off and ignore the contents
  string st0 = st;
  string::size_type k = st0.find_first_of("#");
  if (k != string::npos) st0 = st0.substr(0, k);
  
  string::size_type i = 0;
  while ( true )
    {
      // Cut off leading spaces
      while(i < st0.length() && st[i] == ' ') i++;
      if (i == st0.length()) return;
      if(memoryType == MEM_INST)
      {
        tokens.push_back(st0);
        return;
      }
      string::size_type k = st0.find_first_of(" ", i);
      if (k == string::npos)
        {
          tokens.push_back(st0.substr(i, string::npos));
          return;
        }
      else
        {
          tokens.push_back(st0.substr(i, k - i));
          i = k + 1;
          if (i == st0.length()) return;
        }
    }
  return;
}

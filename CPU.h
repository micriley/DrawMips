// Draw and animate a computer internals
// Michael K. Riley, Georgia Tech, Summer 2008

#ifndef __CPU__
#define __CPU__

#include <string>
#include <qrect.h>

#include "Memory.h"

class QDisplay;
class Computer;

class CPU
{
public:
  CPU(QDisplay& d0, Computer& c);
  void InitRegisters(unsigned n);  // Number of registers
  void Redraw(QPainter&);          // Redraw for animatin
  void ExecuteNextInstruction();// Read and execute  next instruction
  void SetPC(int);       // Set PC Value VIA OFFSET
  int GetPC();                  // Get PC Value Utility
  int GetRegValueInt(unsigned);     // Get the integer value of specified Register
  unsigned GetRegValueIntU(unsigned);     // Get the unsigned integer value of specified Register
  long GetRegValueLong(unsigned);    // Get the long value of specified, for MUL and DIV
  unsigned long GetRegValueLongU(unsigned);    // Get the unsigned long value of specified, for MUL and DIV
  Contents GetRegValue(unsigned);
private:
  void StartAnim3();                     // Setup the variables for 3-seg anim
  void StartAnimALU(int,int,int,std::string,std::string,std::string);
  void FinishAnim();
  int  offsetToLabel(std::string label);
public:
  QDisplay& d;
  Computer& c;
  Memory    regMem;      // Memory object for registers
  Memory*   regSourceObject; //Computer drawing references
  Memory*   regTargetObject; //Computer drawing references
  Memory    pcMem;       // Memory object for Program Counter
  Memory    ciMem;       // Memory object for current instruction
  QRect     cpuRect;     // Rectangle for CPU
  QRect     titleRect;   // Rect for CPU TItle
  QRect     regTitleRect;// Rect for "Registers"
  QRect     ciTitleRect; // Rect or "Current Instruction"
  QRect     pcTitleRect; // Rect for "PC"
  int       pc;
  bool      halted;
};

#endif

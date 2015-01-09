// Class representing the entire computer, CPU, memory, IO, etc
// George F. Riley, Georgia Tech, Summer 2003

#ifndef __COMPUTER__
#define __COMPUTER__

#include <queue>
#include <QWidget>

#include "CPU.h"
#include "Memory.h"
#include "ALU.h"
#include "TimeButton.h"

class QDisplay;

// Number of animation steps
#define NSTEPS 60
// NSteps3 is for 3-segment animation
#define NSTEPS3 20
#define NSTEPSA 40
#define NSTEPSB 40
class RegisterTarget
{
public:
  Memory* regObject;
  unsigned regNumber;
};

class Computer : public QWidget
{
public:
  typedef enum 
  {
    NONE,
    START_ANIMATION, ANIMATING, END_ANIMATION,
    START_ANIMATION3, ANIMATING3, END_ANIMATION3,
    START_ALUREAD,ALUREAD,END_ALUREAD,
    START_ALUWRITE,ALUWRITE,END_ALUWRITE,
    START_RNI, RNI, END_RNI,
    START_READ, END_READ, START_WRITE, START_WRITE2, END_WRITE,
  } StepTypes_t;
  typedef struct
  {
    QPoint         anim3; // Last starting ending point for erase
    QPoint         anim3_0;  // Start of 3 segment animation
    QPoint         anim3_1;
    QPoint         anim3_2;
    QPoint         anim3_3;
    std::string    anim0String;
    Memory*        regTargetObject;
    unsigned       regTarget;
    Contents       writeContents;
  } Anim3Struct;
  typedef struct
  {
    QPoint        animALUINA_0;
    QPoint        animALUINA_1;
    QPoint        animALUINA_2;
    QPoint        animALUINA_3;
    QPoint        animALUINA_4;
    QPoint        animALUINB_0;
    QPoint        animALUINB_1;
    QPoint        animALUINB_2;
    QPoint        animALUINB_3;
    QPoint        animALUINB_4;
    QPoint        animALUOUT_0;
    QPoint        animALUOUT_1;
    QPoint        animALUOUT_2;
    QPoint        animALUOUT_3;
    QPoint        animALUOUT_4;
    std::string   AString;
    std::string   BString;
    std::string   OutString;
    Memory*       regTargetObject;
    unsigned      regTarget;
    Contents      writeContents;
  } AnimALUStruct;
public:
  Computer(QDisplay&,
           const std::string& input,
           unsigned nReg,
					 int fs = 1);
  void LoadDataMem(const std::string& fileName,
            const std::string& t1 = std::string(),    // Title1
            const std::string& t2 = std::string());   // Title2
  void LoadInstMem(const std::string& fileName,
            const std::string& t1 = std::string(),    // Title1
            const std::string& t2 = std::string());   // Title2
  void ReadMem(Memory& from, unsigned a1, Memory& to, unsigned a2);
  void WriteMem(Memory& from, unsigned a1, Memory& to, unsigned a2);
  void Execute(); // Execute the program
  void Step();   // For animations
  void Redraw(QPainter& p); // For animations
  void ComputerBox(QPainter& p);  // Draw large box around the Computer
  void ReadMem(const MemoryLocation& from, MemoryLocation to);
  void pushAnim3DrawQueue();
  void pushAnimALUDrawQueue();
private:
  void pullAnim3DrawQueue();
  void pullAnimALUDrawQueue();
public:
  QDisplay& d;     // Screen display object
  Memory    data;  // Data memory
  Memory    inst;  // Instruction Memory
  CPU       cpu;   // CPU Element
  ALU       myALU; // ALU struct for drawing and animation calls
  //TimeButtonController timeController; DON'T WORK
  int frameSkip;
	int SLOW_STEP;
	int FAST_STEP;
	int QUICK_STEP;
  // Common vars for animating
  QPoint    anim0; // Starting point of animation
  QPoint    anim1; // Ending point of animation
  QPoint    anim2; // Return point (may be empty)
  QPoint    anim3; // Last starting ending point for erase
  QPoint    anim3_0;  // Start of 3 segment animation
  QPoint    anim3_1;
  QPoint    anim3_2;
  QPoint    anim3_3;
  QPoint    animALUINA_0;
  QPoint    animALUINA_1;
  QPoint    animALUINA_2;
  QPoint    animALUINA_3;
  QPoint    animALUINA_4;
  QPoint    animALUINB_0;
  QPoint    animALUINB_1;
  QPoint    animALUINB_2;
  QPoint    animALUINB_3;
  QPoint    animALUINB_4;
  QPoint    animALUOUT_0;
  QPoint    animALUOUT_1;
  QPoint    animALUOUT_2;
  QPoint    animALUOUT_3;
  QPoint    animALUOUT_4;
  QPoint    anim3_current;
  QPointF    animALUINA_current;
  QPointF    animALUINB_current;
  QPointF    animALUOUT_current;
  int       anim3_step;      // 0 = left, 1 = down, 2 = right
  int       animALUINA_step; // 0 = left, 1 = down, 2 = left, 3 = up
  int       animALUINB_step; // 0 = left, 1 = down, 2 = left, 3 = up
  int       animALUOUT_step; // 0 = up, 1 = right, 2 = up/down, 4 = right
  // Re-uses amim0string
  bool      animStarting;    // True if advaning endpoint, false if starting
  StepTypes_t stepType;      // Current execution step
  StepTypes_t nextStepType;  // Next step after animation completes
  int      nextStepTime;     // Time for next animation step
  int      animCount;        // A counter for animating
  int      animCountA;
  int      animCountB;
  double   deltaX;
  double   deltaY;
  double   r;
  double   theta;
  std::string     anim0String;
  std::string     anim1String;
  std::string     AString;
  std::string     BString;
  std::string     OutString;
  Memory*         regTargetObject;
  unsigned        regTarget;    // For read
  unsigned        readAddress;  // For read
  Contents        readContents; // For read
  Memory*         regSourceObject;
  unsigned        regSource;    // For write
  unsigned        writeAddress; // For write
  Contents        writeContents;// For write
  std::queue <Anim3Struct> anim3DrawQueue; //For doing multiple anim3 animations
  std::queue <AnimALUStruct> animALUDrawQueue;
};

#endif

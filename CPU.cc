// Draw and animate a computer internals
// Michael K. Riley, Georgia Tech, Summer 2008

#include <string>
#include <math.h>
#include <iostream>
#include <sstream>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include <qpainter.h>

#include "CPU.h"
#include "RectHelpers.h"
#include "Memory.h"
#include "Computer.h"
#include "Instruction.h"

#include "qdisplay.h"

#define SSTR( x ) dynamic_cast< std::ostringstream & >(( std::ostringstream() << std::dec << x ) ).str()
#define CPU_NORMAL_X_POS  0.3125
#define CPU_NORMAL_Y_POS	0.0625
#define CPU_NORMAL_WIDTH  0.375
#define CPU_NORMAL_HEIGHT 0.875
  
CPU::CPU(QDisplay& d0, Computer& c0):d(d0),c(c0),regMem(MEM_REG),pcMem(),ciMem(MEM_INST),pc(0),halted(false)
{
  cpuRect = QRect(d.width()*CPU_NORMAL_X_POS, // x
                  d.height()*CPU_NORMAL_Y_POS,              // y
                  d.width()*CPU_NORMAL_WIDTH,                  // w
                  d.height()*CPU_NORMAL_HEIGHT + 8);// h
  
  QPainter p(d.Pixmap());
  p.drawRect(cpuRect);
  titleRect = QRect(cpuRect.x(), cpuRect.y() - 16, cpuRect.width(), 16);
  p.drawText(titleRect, Qt::AlignCenter, "CPU");
  d.Update();
  regSourceObject = &regMem;
  regTargetObject = &regMem;
}

void CPU::InitRegisters(unsigned n)
{
  QPainter p(d.Pixmap());
  QPoint where1(cpuRect.x(), cpuRect.y() + 20);
  QRect  size1(where1.x(), where1.y(), cpuRect.width() - 32, 16);
  // Draw the current instruction register first then others below
  ciMem.LoadEmpty(1, where1, size1, true);
  ciMem.Draw(p, where1, size1, false, "", "IR");
  
  // Now program counter
  QPoint where2(where1);
  QRect  size2(size1);
  where2.setY(where2.y() + size1.height() + 8);
  size2.moveTo(where2);
  pcMem.LoadEmpty(1, where2, size2, true);
  pcMem.Draw(p, where2, size2, false, "", "PC");
  // Set the initial PC to the first address of inst memory
  SetPC(c.inst.firstAddress - c.inst.addressAdder);
  // Now registers
  QPoint where3(where2);
  QRect size3(size2);
	where3.setX(where2.x());
  where3.setY(where3.y() + size2.height());
  size2.moveTo(where3);
  regTitleRect = p.boundingRect(size2, Qt::AlignCenter, "Registers");
  size3.setWidth(size3.width() - 12);
  regMem.LoadEmpty(n, where3, size3, true);
  regMem.Draw(p, where3, size3, true, "Registers");

  d.Update();
}

void CPU::Redraw(QPainter& p)
{
  p.drawRect(cpuRect);
  p.drawText(titleRect, Qt::AlignCenter, "CPU");
  // Below are not needed since the memory->Redraw does this
  //p.drawText(regTitleRect, Qt::AlignCenter, "Registers");
  //p.drawText(ciTitleRect,  Qt::AlignCenter, "Current Instruction");
  //p.drawText(pcTitleRect,  Qt::AlignCenter, "PC");
  ciMem.Redraw(p);
  pcMem.Redraw(p);
  regMem.Redraw(p);
}

void CPU::SetPC(int p0)
{
  pc = pc + p0;
  pcMem.SetContentsInt(pc, 0);
}

int CPU::GetPC(){
  return pcMem.GetContentsInt(0);
}

int CPU::GetRegValueInt(unsigned regNum)
{
 Contents data = regMem.GetContents(regNum);
 int v = data.cType == CT_UINT ? regMem.GetContents(regNum).ui: regMem.GetContents(regNum).i;
 return v;
}

unsigned CPU::GetRegValueIntU(unsigned regNum)
{
 //NEED to change this make make sure it works like the other GetRegValueInt command
 unsigned v = regMem.GetContents(regNum).i;
 return v;
}

long CPU::GetRegValueLong(unsigned regNum)
{
 long v = (long)regMem.GetContents(regNum).i;
 return v;
}

unsigned long CPU::GetRegValueLongU(unsigned regNum)
{
 unsigned long v = (unsigned long)regMem.GetContents(regNum).i;
 return v;
}
 
Contents CPU::GetRegValue(unsigned regNum)
{
  return regMem.GetContents(regNum);
}

//Helpers for shifting
static int logicalRightShift(int source, unsigned shiftNum)
{
  int64_t mask = 1;
  for(unsigned i = 0; i < 64 - shiftNum; i++)
  {
    mask = mask << 1;
    mask++;
  }
  return (source >> shiftNum) & mask;
}

static int64_t logicalRightShiftL(int64_t source, unsigned shiftNum)
{
  int64_t mask = 1;
  for(unsigned i = 0; i < 64 - shiftNum; i++)
  {
    mask = mask << 1;
    mask++;
  }
  return (source >> shiftNum) & mask;
}

static uint64_t logicalRightShiftUL(uint64_t source, unsigned shiftNum)
{
  uint64_t mask = 1;
  for(unsigned i = 0; i < 64 - shiftNum; i++)
  {
    mask = mask << 1;
    mask++;
  }
  return (source >> shiftNum) & mask;
}

void CPU::ExecuteNextInstruction()
{ // Reads, decodes, and executes the next instruction
  //NOTE: Refactor
  MemoryLocation* ml = ciMem.GetLocation(0);
  std::string currentInstString = ciMem.GetContentsString(0);
  Instruction currentInst(currentInstString,c.data,c.inst,ml->sourceLine);
  c.stepType = Computer::RNI;   // If no animation needed for instruction
  c.nextStepTime = d.msTime + c.SLOW_STEP; // If no animation
  unsigned reg0;
  unsigned reg1;
  unsigned reg2;
  Contents reg0v;       // Register values
  Contents reg1v;
  Contents reg2v;
  int reg0vi;      // Register values (integer)
  int reg1vi;
  int reg2vi;
  int const1;
  long reg0vl;
  long reg1vl;
  int64_t result;          // Utility Variables For MUL/DIV
  int64_t upper;
  int64_t lower;
  unsigned mod;
  int wordData;         // Word Data for Load/Store
  Contents newData;
  unsigned uWordData;
  unsigned upperdata;   // LUI Utility
  unsigned lowerdata;
  unsigned luiData;
  unsigned Uconst1; // SRA Utility and signed values
  unsigned Ureg0vi;
  unsigned Ureg1vi;
  unsigned Ureg2vi;
  unsigned Uresult;
  uint64_t Uupper;
  uint64_t Ulower;
  Contents dataContents;
  std::ostringstream oss;
  string contents;
  switch (currentInst.instruction.instruction) {
  case ADD:          // Add R1+R2, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi + reg2vi,true);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR(reg1vi+reg2vi));
    break;
  case ADDU:          // Add R1+R2, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    Ureg1vi = GetRegValueIntU(reg1);
    Ureg2vi = GetRegValueIntU(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ureg1vi + Ureg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(Ureg1vi),SSTR(Ureg2vi),SSTR(Ureg1vi+Ureg2vi));
    //regMem.SetContentsInt(p, reg1vi + reg2vi, reg0);
  break;
  case SUB:          // Subtract R1-R2, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi - reg2vi,true);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR(reg1vi-reg2vi));
  break;
  case SUBU:          // Subtract R1-R2, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    Ureg1vi = GetRegValueIntU(reg1);
    Ureg2vi = GetRegValueIntU(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ureg1vi - Ureg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(Ureg1vi),SSTR(Ureg2vi),SSTR(Ureg1vi-Ureg2vi));
    //regMem.SetContentsInt(p, reg1vi - reg2vi, reg0);
  break;
  case ADDI:         // Add R1 + I, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    const1 = currentInst.GetConst(2);
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi + const1,true);
    //Continue from here
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(const1),SSTR(reg1vi+const1));
  break;
  case ADDIU:         // Add R1 + I, store in R0
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Uconst1 = currentInst.GetConst(2);
    Ureg1vi = GetRegValueIntU(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ureg1vi + Uconst1,false);
    StartAnim3();
    StartAnimALU(reg1,reg1,reg0,SSTR(Ureg1vi),SSTR(Uconst1),SSTR(Ureg1vi+Uconst1));
  break;
  case SLT:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi < reg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR((reg1vi < reg2vi)));
  break;
  case SLTI:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    const1 = currentInst.GetConst(2);
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi < const1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(const1),SSTR((reg1vi < const1)));
  break;
  case SLTU:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    Ureg1vi = GetRegValueInt(reg1);
    Ureg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ureg1vi < Ureg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(Ureg1vi),SSTR(Ureg2vi),SSTR((Ureg1vi < Ureg2vi)));
  break;
  case SLTIU:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Uconst1 = currentInst.GetConst(2);
    Ureg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ureg1vi < Uconst1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(Ureg1vi),SSTR(Uconst1),SSTR((Ureg1vi < Uconst1)));
  break;
  case MULT:          // Multiply R0 * R1, store in HI, LO
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg0vl = GetRegValueLong(reg0);
    reg1vl = GetRegValueLong(reg1);
    result = reg0vl * reg1vl;
    upper = result & 0xFFFFFFFF00000000L;
    upper = logicalRightShiftL(upper,32);
    lower = result & 0x00000000FFFFFFFFL;
    c.regTargetObject = &regMem;
    c.regTarget = 32; // 0 = HI register
    c.writeContents = Contents();
    c.writeContents.SetInt(upper,false);
    StartAnimALU(reg0,reg1,32,SSTR(reg0vl),SSTR(reg1vl),SSTR(upper));
    c.regTarget = 33; // 1 = LO register
    c.writeContents = Contents();
    c.writeContents.SetInt(lower,false);
    StartAnimALU(reg0,reg1,33,SSTR(reg0vl),SSTR(reg1vl),SSTR(lower));
  break;
  case MULTU:          // Multiply R0 * R1, store in HI, LO
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Ureg0vi = GetRegValueIntU(reg0);
    Ureg1vi = GetRegValueIntU(reg1);
    Uresult = (long)Ureg0vi * (long)Ureg1vi;
    Uupper = Uresult & 0xFFFFFFFF00000000;
    Uupper = logicalRightShiftUL(Uupper,32);
    Ulower = Uresult & 0x00000000FFFFFFFF;
    c.regTargetObject = &regMem;
    c.regTarget = 32;
    c.writeContents = Contents();
    c.writeContents.SetInt(Uupper,false);
    StartAnimALU(reg0,reg1,32,SSTR(Ureg0vi),SSTR(Ureg1vi),SSTR(Uupper));
    c.regTarget = 33;
    c.writeContents = Contents();
    c.writeContents.SetInt(Ulower,false);
    StartAnimALU(reg0,reg1,33,SSTR(Ureg0vi),SSTR(Ureg1vi),SSTR(Ulower));
  break;
  case DIV:          // Devide R0 / R1, store in HI, LO
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg0vi = GetRegValueInt(reg0);
    reg1vi = GetRegValueInt(reg1);
    result = reg0vi / reg1vi;
    mod = reg0vi % reg1vi;
    c.regTargetObject = &regMem;
    c.regTarget = 32;
    c.writeContents = Contents();
    c.writeContents.SetInt(mod,false);
    StartAnimALU(reg0,reg1,32,SSTR(reg0vi),SSTR(reg1vi),SSTR(mod));
    c.regTarget = 33;
    c.writeContents = Contents();
    c.writeContents.SetInt(result,true);
    StartAnimALU(reg0,reg1,33,SSTR(reg0vi),SSTR(reg1vi),SSTR(result));
  break;
  case DIVU:          // Devide R0 / R1, store in HI, LO
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Ureg0vi = GetRegValueInt(reg0);
    Ureg1vi = GetRegValueInt(reg1);
    result = Ureg0vi / Ureg1vi;
    mod = Ureg0vi % Ureg1vi;
    c.regTarget = 32;
    c.writeContents = Contents();
    c.writeContents.SetInt(mod,false);
    StartAnimALU(reg0,reg1,32,SSTR(Ureg0vi),SSTR(Ureg1vi),SSTR(mod));
    c.regTarget = 33;
    c.writeContents = Contents();
    c.writeContents.SetInt(result,true);
    StartAnimALU(reg0,reg1,33,SSTR(Ureg0vi),SSTR(Ureg1vi),SSTR(result));
  break;
  case MFHI:
    reg0 = currentInst.GetReg(0);
    c.regSource = 32;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(regMem.GetContentsInt(32),false);
    StartAnim3();
  break;
  case MFLO:
    reg0 = currentInst.GetReg(0);
    c.regSource = 32;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(regMem.GetContentsInt(33),false);
    StartAnim3();
  break;
  case LUI:
    reg0 = currentInst.GetReg(0);
    reg0vi = GetRegValueInt(reg0);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    //Need a way to just load a constant into mem animation
    upperdata = (const1 << 16);
    lowerdata = (reg0vi << 16) >> 16;
    luiData = upperdata & lowerdata;
    dataContents = Contents();
    dataContents.SetInt(luiData,false);
    //LUI should see the "upper bits" going to the register
    newData = Contents();
    newData.SetInt(upperdata,false);
    c.anim1String = newData.GetString();
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.readContents = dataContents;
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
  break;
  case AND:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi & reg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR((reg1vi & reg2vi)));
  break;
  case ANDI:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    const1 = currentInst.GetConst(2);
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi & const1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(const1),SSTR((reg1vi & const1)));
  break;
  case OR:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi | reg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR((reg1vi | reg2vi)));
  break;
  case ORI:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    const1 = currentInst.GetConst(2);
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi | const1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(const1),SSTR((reg1vi | const1)));
  break;
  case NOR:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(!(reg1vi | reg2vi),false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR(!(reg1vi & reg2vi)));
  break;
  case XOR:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    reg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(reg2vi),SSTR((reg1vi ^ reg2vi)));
    StartAnim3();
  break;
  case XORI:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg1vi = GetRegValueInt(reg1);
    const1 = currentInst.GetConst(2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(const1),SSTR((reg1vi ^ const1)));
    StartAnim3();
  break;
  case SLL:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Uconst1 = currentInst.GetConst(2);
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi<< Uconst1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(Uconst1),SSTR((reg1vi << Uconst1)));
  break;
  case SLLV:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    Ureg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi<< Ureg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(Ureg2vi),SSTR((reg1vi << Ureg2vi)));
  break;
  case SRL:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Uconst1 = currentInst.GetConst(2);
    Ureg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(logicalRightShift(Ureg1vi, Uconst1),false);
    StartAnimALU(reg1,reg1,reg0,SSTR(Ureg1vi),SSTR(Uconst1),SSTR(logicalRightShift(Ureg1vi, Uconst1)));
  break;
  case SRLV:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    Ureg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(logicalRightShift(reg1vi, Ureg2vi),false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(Ureg2vi),SSTR(logicalRightShift(reg1vi, Ureg2vi)));
  case SRA:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    Uconst1 = currentInst.GetConst(2);
    //We're not using unsigned here because of arithmetic shifting
    reg1vi = GetRegValueInt(reg1);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi >> Uconst1,false);
    StartAnimALU(reg1,reg1,reg0,SSTR(reg1vi),SSTR(Uconst1),SSTR((reg1vi >> Uconst1)));
  break;
  case SRAV:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg2 = currentInst.GetReg(2);
    reg1vi = GetRegValueInt(reg1);
    //We're not using unsigned here because of arithmetic shifting
    Ureg2vi = GetRegValueInt(reg2);
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.writeContents = Contents();
    c.writeContents.SetInt(reg1vi >> Ureg2vi,false);
    StartAnimALU(reg1,reg2,reg0,SSTR(reg1vi),SSTR(Ureg2vi),SSTR((reg1vi >> Ureg2vi)));
  break;
  case LW:
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    reg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
		else if(currentInst.operands[1].opType == OP_InstructionTag)
		{
			const1 = currentInst.m.FindAddressTag(currentInst.operands[1].Address);
			if(const1 == -1)
			{
				cout << "LW: Address Tag Not Found" << endl;
				exit(1);
			}
		}
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.readAddress = reg2vi + const1;
    oss << c.readAddress;
    contents = oss.str();
    c.anim0String = contents;
    c.anim1String = c.data.GetContentsString(c.readAddress);
    c.regTarget = reg0;
    c.readContents = c.data.GetContents(c.readAddress);
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
  break;
  case SW:          // Store R1, address in R2
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    reg0vi = GetRegValueInt(reg0);
    reg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.writeAddress = const1 + reg2vi;
    // Animate the write
    c.regSource = reg0;
    oss << c.writeAddress;
    c.anim0String = oss.str();
    c.anim1String = regMem.GetContentsString(reg0);
    c.writeContents = regMem.GetContents(reg0);
    c.stepType = Computer::START_WRITE;
    c.nextStepTime = d.msTime + c.FAST_STEP;
  break;
  case LH:
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    reg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.readAddress = reg2vi + const1;
    wordData = c.data.GetContentsInt(c.readAddress);
    //Bit Masking
    wordData = wordData << 16;
    wordData = wordData >> 16;
    oss << c.readAddress;
    c.anim0String = oss.str();
    oss.clear();
    oss << wordData;
    c.anim1String = oss.str();
    c.regTarget = reg0;
    newData = Contents();
    newData.SetInt(wordData,true);
    oss << wordData;
    contents = oss.str();
    c.anim0String = contents;
    c.readContents = newData;
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.readContents = c.data.GetContents(c.readAddress);
  break;
  case LB:
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    reg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.readAddress = reg2vi + const1;
    wordData = c.data.GetContentsInt(c.readAddress);
    //Bit Masking
    wordData = wordData << 24;
    wordData = wordData >> 24;
    oss << c.readAddress;
    c.anim0String = oss.str();
    oss.clear();
    oss << wordData;
    c.anim1String = oss.str();
    c.regTarget = reg0;
    newData = Contents();
    newData.SetInt(wordData,true);
    oss << wordData;
    contents = oss.str();
    c.anim0String = contents;
    c.readContents = newData;
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.readContents = c.data.GetContents(c.readAddress);
  break;
  case LBU:
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    Ureg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.readAddress = Ureg2vi + const1;
    uWordData = c.data.GetContents(c.readAddress).GetInt();
    //Bit Masking
    uWordData = uWordData << 24;
    uWordData = uWordData >> 24;
    oss << c.readAddress;
    c.anim0String = oss.str();
    oss.clear();
    oss << uWordData;
    c.anim1String = oss.str();
    c.regTarget = reg0;
    newData = Contents();
    newData.SetInt(uWordData,false);
    c.readContents = newData;
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
    c.regTargetObject = &regMem;
    c.regTarget = reg0;
    c.readContents = c.data.GetContents(c.readAddress);
    c.stepType = Computer::START_READ;
    c.nextStepTime = d.msTime + c.FAST_STEP;
  break;
  case SB:          // Store R1, address in R2
    reg0 = currentInst.GetReg(0);
    reg2 = currentInst.GetReg(2);
    reg0vi = GetRegValueInt(reg0);
    reg2vi = GetRegValueInt(reg2);
    if(currentInst.operands[1].opType == OP_NONE)
    {
      const1 = 0;
    }
    else if(currentInst.operands[1].opType == OP_Constant)
    {
        const1 = currentInst.GetConst(1);
    }
    else
    {
      cout << "OOps, expected Const type for operand 1"
           << " found " << currentInst.operands[1].opType 
           << "On Instruction" << currentInst.instruction.st
           << endl;
      exit(1);
    }
    c.writeAddress = const1 + reg2vi;
    // Animate the write
    c.regSource = reg0;
    c.anim1String = regMem.GetContentsString(reg0);
    wordData = regMem.GetContents(reg0).GetInt();
    wordData = wordData << 24;
    wordData = wordData >> 24;
    oss << wordData;
    c.anim0String = oss.str();
    oss.clear();
    oss << c.writeAddress;
    c.anim1String = oss.str();
    newData = Contents();
    newData.SetInt(wordData,true);
    c.readContents = newData;
    c.stepType = Computer::START_WRITE;
    c.nextStepTime = d.msTime + c.FAST_STEP;
  break;
  case J:
    const1 = currentInst.GetOptype(0) == OP_AddressTag ? offsetToLabel(currentInst.GetAddress(0)) :currentInst.GetConst(0);
    SetPC(const1);
  break;
  case JR:
    reg0 = currentInst.GetReg(0);
		if(reg0 == 31) c.stepType = Computer::NONE;
		else SetPC(reg0);
  break;
  case JAL:
    const1 = currentInst.GetOptype(0) == OP_AddressTag ? offsetToLabel(currentInst.GetAddress(0)) : currentInst.GetConst(0);
    c.cpu.regTargetObject = &regMem;
    c.regTarget = 31;
    c.writeContents = Contents();
    c.writeContents.SetInt(GetPC(),true);
    StartAnim3();
    SetPC(const1);
  break;
  case BEQ:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg0vi = GetRegValueInt(reg0);
    reg1vi = GetRegValueInt(reg1);
    const1 = currentInst.GetOptype(2) == OP_InstructionTag ? offsetToLabel(currentInst.GetAddress(0)) :currentInst.GetConst(2);
    if (reg0vi == reg1vi) SetPC(const1);
  break;
  case BNE:
    reg0 = currentInst.GetReg(0);
    reg1 = currentInst.GetReg(1);
    reg0vi = GetRegValueInt(reg0);
    reg1vi = GetRegValueInt(reg1);
    const1 = currentInst.GetOptype(2) == OP_InstructionTag ? offsetToLabel(currentInst.GetAddress(2)) :currentInst.GetConst(2);
    if (reg0vi != reg1vi) SetPC(const1);
  break;
  }
  FinishAnim();
}

void CPU::StartAnim3()
{
  // Amimate 3-segment
  c.anim3_0 = ciMem.GetLeftCenter(0);
  c.anim3_1 = c.anim3_0 + QPoint(-60, 0);
  c.anim3_3 = c.regTargetObject->GetLeftCenter(c.regTarget);
  c.anim3_2 = c.anim3_3;
  c.anim3_2.setX(c.anim3_1.x());
  c.anim0String = c.writeContents.GetString();
  c.stepType = Computer::START_ANIMATION3;
  c.pushAnim3DrawQueue();
}

void CPU::StartAnimALU(int regA,int regB,int regOut,std::string aString,std::string bString,std::string outString)
{
  //Animation points for the A line
  c.animALUINA_0 = regMem.GetPointOnMemLoc(regA,0,0.25);
  c.animALUINA_1 = c.animALUINA_0;
  c.animALUINA_1.setX(c.animALUINA_1.x() + (c.myALU.boundingRect().bottomRight().x() - c.animALUINA_0.x()) * 0.25);
  c.animALUINA_2 = c.animALUINA_1;
  c.animALUINA_2.setY(c.myALU.boundingRect().bottomRight().y() + 50);
  c.animALUINA_3 = c.animALUINA_2;
  c.animALUINA_3.setX(c.myALU.getAPoint().x());
  c.animALUINA_4 = c.myALU.getAPoint();
  //Animation points for the B line
  c.animALUINB_0 = regMem.GetPointOnMemLoc(regB,0,0.75);
  c.animALUINB_1 = c.animALUINB_0;
  c.animALUINB_1.setX(c.animALUINB_1.x() + (c.myALU.boundingRect().bottomRight().x() - c.animALUINB_1.x()) * 0.50);
  c.animALUINB_2 = c.animALUINB_1;
  c.animALUINB_2.setY(c.myALU.boundingRect().bottomRight().y() + 75);
  c.animALUINB_3 = c.animALUINB_2;
  c.animALUINB_3.setX(c.myALU.getBPoint().x());
  c.animALUINB_4 = c.myALU.getBPoint();
  //Animation points for the Out Line
  c.animALUOUT_0 = c.myALU.getOutPoint();
  c.animALUOUT_1 = c.animALUOUT_0;
  c.animALUOUT_1.setY(c.animALUOUT_0.y() - 30);
  c.animALUOUT_2 = c.animALUOUT_1;
  c.animALUOUT_2.setX(c.animALUOUT_2.x() + 0.75*(regMem.GetPointOnMemLoc(regOut,0,0.5).x() - c.animALUOUT_2.x()));
  c.animALUOUT_3 = c.animALUOUT_2;
  c.animALUOUT_3.setY(regMem.GetPointOnMemLoc(regOut,0,0.5).y());
  c.animALUOUT_4 = regMem.GetPointOnMemLoc(regOut,0,0.5);
  //Miscellaneous textline setups and such
  c.AString = aString;
  c.BString = bString;
  c.OutString = outString;
  c.stepType = Computer::START_ALUREAD;
  c.pushAnimALUDrawQueue();
}

void CPU::FinishAnim()
{
  c.nextStepTime = d.msTime + c.FAST_STEP;
}

int CPU::offsetToLabel(std::string label)
{
  MemoryLocation ml = Memory::labelMap[label];
  return (ml.index*c.inst.addressAdder + c.inst.firstAddress) - pc;
}

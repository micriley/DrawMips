// Model for the complete computer
// George F. Riley, Georgia Tech, Summer 2003

#include <iostream>
#include <sstream>
#include <math.h>

#include <qpainter.h>

#include "Computer.h"
#include "Memory.h"
#include "Parser.h"
#include "RectHelpers.h"
#include "qdisplay.h"
#include "ALU.h"


using namespace std;

Computer::Computer(QDisplay& qd,
                   const string& input,
                   unsigned nreg)
  : d(qd),data(MEM_DATA),inst(MEM_INST,0),cpu(qd, *this),myALU(), timeController(this),frameSkip(1), stepType(NONE), nextStepType(NONE), animCount(0)
{
  QPainter p(d.Pixmap());
  QPoint dataPoint  = QPoint(16, 32);
  QRect  dataRect   = QRect(0,0,64,15);
  data.Load(dataPoint, dataRect,true);
  QPoint instPoint  = QPoint(d.width() - 176, 32);
  QRect  instRect   = QRect(0,0,112,16);
  inst.Load(instPoint, instRect,true);
  Parser parse(input);
  parse.ReadMemoryContents(data,inst);
  data.Draw(p,dataPoint,dataRect,true,"Data","Memory");
  inst.Draw(p,instPoint,instRect,true,"Instruction","Memory");
  //Draw(p, where, size, useAddr, t1, t2);
  ComputerBox(p);
  myALU.draw(p);
  p.end();
  cpu.InitRegisters(nreg);
  d.Update();
}

void Computer::Execute()
{
}

void Computer::Step()
{
  int deltaX;
  int deltaY;
  if (nextStepTime > d.msTime || d.paused) return; // Not time
  QPainter p(d.Pixmap());
  p.setRenderHint(QPainter::Antialiasing);
  switch(stepType) 
    {
    case NONE:
      break;
    case START_ANIMATION:
      deltaX = (double)anim1.x() - anim0.x();
      deltaY = (double)anim1.y() - anim0.y();
      r = sqrt(deltaX * deltaX + deltaY * deltaY);
      theta = atan2(deltaY, deltaX);
      stepType = ANIMATING;
      anim3 = QPoint();
      animStarting = true;
      animCount = 0; 
      // Fall through
    case ANIMATING:
      Redraw(p);
      if (animStarting)
        { // draw from anim0 to moving endpoint
          animCount += frameSkip;
          double thisR = r * animCount / NSTEPS;
          if(0)cout << "Animating, animcount " << animCount 
               << " a0.x " << anim0.x() << " a0.y " << anim0.y()
               << " a1.x " << anim1.x() << " a1.y " << anim1.y()
               << " theta " << theta
               << " r " << r << " this r " << thisR
               << endl;
          // Now find intermediate x and y
          double x = anim0.x() + thisR * cos(theta);
          double y = anim0.y() + thisR * sin(theta);
          anim3 = QPoint((int)x, (int)y);
          p.drawLine(anim0, anim3);
          if (!anim0String.empty())
            {
              QRect stRect(anim3, QSize(0,0));
              stRect = p.boundingRect(stRect, Qt::AlignCenter, anim0String.c_str());
              p.drawText(stRect, Qt::AlignCenter, anim0String.c_str());
            }
          nextStepTime = d.msTime + QUICK_STEP;
          if (animCount == (NSTEPS-1))
            {
              animStarting = false;
              animCount = 0;
              animCount = (NSTEPS-2); // ! Try this. no shrinking line
            }
        }
      else
        {
          animCount++;
          double thisR = r * animCount / NSTEPS;
          // Now find intermediate x and y
          double x = anim0.x() + thisR * cos(theta);
          double y = anim0.y() + thisR * sin(theta);
          anim3 = QPoint((int)x, (int)y);
          p.drawLine(anim3, anim1);
          nextStepTime = d.msTime + QUICK_STEP;
          if (animCount == (NSTEPS-1))
            { // See if a return point specified, if so start the return
              if (anim2 != QPoint())
                {
                  animStarting = true;
                  animCount = 0;
                  stepType = START_ANIMATION;
                  anim0 = anim1;
                  anim1 = anim2;
                  anim2 = QPoint();
                  anim0String = anim1String;
                }
              else
                {
                  stepType = END_ANIMATION;
                }
            }
        }
      break;
    case END_ANIMATION:
      Redraw(p);
      stepType = nextStepType;
      break;
    case START_ALUREAD:
    {
      animCountA      = 0;
      animCountB      = 0;
      animALUINA_step = 0;
      animALUINB_step = 0;
      stepType        = ALUREAD;
      break;
    }
    case ALUREAD:
    {
      //Redraw, increment counters, and check if we change drawlines
      Redraw(p);
      animCountA++;
      animCountB++;
      if(animCountA == NSTEPSA)
      {
        animCountA = 0;
        animALUINA_step++;
      }
      if(animCountB == NSTEPSB)
      {
        animCountB = 0;
        animALUINB_step++;
      }
      //Break out at end
      if(animALUINA_step > 3 && animALUINB_step > 3)
      {
        stepType = END_ALUREAD;
        break;
      }
      //Draw the A lines we've allready done
      p.setPen(Qt::red);
      if (animALUINA_step > 0) p.drawLine(animALUINA_0,animALUINA_1);
      if (animALUINA_step > 1) p.drawLine(animALUINA_1,animALUINA_2);
      if (animALUINA_step > 2) p.drawLine(animALUINA_2,animALUINA_3);
      //Draw the current A line
      switch(animALUINA_step)
      {
      case 0:
        {
          // Moving left
          deltaX = animALUINA_1.x() - animALUINA_0.x();
          animALUINA_current = animALUINA_0;
          animALUINA_current.setX(animALUINA_0.x() + deltaX / (NSTEPSA) * animCountA);
          p.drawLine(animALUINA_0, animALUINA_current);
          break;
        }
      case 1:
        {
          // Moving down
          deltaY = animALUINA_2.y() - animALUINA_1.y();
          animALUINA_current = animALUINA_1;
          animALUINA_current.setY(animALUINA_1.y() + deltaY / (NSTEPSA) * animCountA);
          p.drawLine(animALUINA_1, animALUINA_current);
          break;
        }
      case 2:
        {
          // Moving left
          deltaX = animALUINA_3.x() - animALUINA_2.x();
          animALUINA_current = animALUINA_2;
          animALUINA_current.setX(animALUINA_2.x() + deltaX / (NSTEPSA) * animCountA);
          p.drawLine(animALUINA_2, animALUINA_current);
          break;
        }
      case 3:
        {
          // Moving up
          deltaY = animALUINA_4.y() - animALUINA_3.y();
          animALUINA_current = animALUINA_3;
          animALUINA_current.setY(animALUINA_3.y() + deltaY / (NSTEPSA) * animCountA);
          p.drawLine(animALUINA_3, animALUINA_current);
          break;
        }
      }
      p.drawText(animALUINA_current,QString::fromStdString(AString));
      //Draw the B lines we've allready done
      p.setPen(Qt::blue);
      if (animALUINB_step > 0) p.drawLine(animALUINB_0,animALUINB_1);
      if (animALUINB_step > 1) p.drawLine(animALUINB_1,animALUINB_2);
      if (animALUINB_step > 2) p.drawLine(animALUINB_2,animALUINB_3);
      //Draw the current B line
      switch(animALUINB_step)
      {
      case 0:
        {
          // Moving left
          deltaX = animALUINB_1.x() - animALUINB_0.x();
          animALUINB_current = animALUINB_0;
          animALUINB_current.setX(animALUINB_0.x() + deltaX / (NSTEPSB) * animCountB);
          p.drawLine(animALUINB_0, animALUINB_current);
        break;
        }
      case 1:
        {
          // Moving down
          deltaY = animALUINB_2.y() - animALUINB_1.y();
          animALUINB_current = animALUINB_1;
          animALUINB_current.setY(animALUINB_1.y() + deltaY / (NSTEPSB) * animCountB);
          p.drawLine(animALUINB_1, animALUINB_current);
          break;
        case 2:
          // Moving right
          deltaX = animALUINB_3.x() - animALUINB_2.x();
          animALUINB_current = animALUINB_2;
          animALUINB_current.setX(animALUINB_2.x() + deltaX / (NSTEPSB) * animCountB);
          p.drawLine(animALUINB_2, animALUINB_current);
          break;
        }
      case 3:
        {
        // Moving up
          deltaY = animALUINB_4.y() - animALUINB_3.y();
          animALUINB_current = animALUINB_3;
          animALUINB_current.setY(animALUINB_3.y() + deltaY / (NSTEPSB) * animCountB);
          p.drawLine(animALUINB_3, animALUINB_current);
          break;
        }
      }
      p.drawText(animALUINB_current,QString::fromStdString(BString));
      p.setPen(Qt::black);
    break;
    }
    case END_ALUREAD:
    {
      Redraw(p);
      stepType = START_ALUWRITE;
      break;
    }
    case START_ALUWRITE:
    {
      animCount = 0;
      animALUOUT_step = 0;
      animALUOUT_current = anim3_0;
      if(animALUDrawQueue.empty())
      {
        nextStepTime = d.msTime + SLOW_STEP;
        stepType = START_RNI;
        break;
      }
      pullAnimALUDrawQueue();
      stepType = ALUWRITE;
    break;
    }
    case ALUWRITE:
    {
      Redraw(p);
      animCount++;
      if (animCount == NSTEPS3)
        {
          animCount = 0;
          animALUOUT_step++;
        }
      if (animALUOUT_step > 3)
        { // Done
          stepType = END_ALUWRITE;
          break;
        }
      if (animALUOUT_step > 0) p.drawLine(animALUOUT_0, animALUOUT_1);
      if (animALUOUT_step > 1) p.drawLine(animALUOUT_1, animALUOUT_2);
      if (animALUOUT_step > 2) p.drawLine(animALUOUT_2, animALUOUT_3);
      if (animALUOUT_step > 3) p.drawLine(animALUOUT_3, animALUOUT_4);
      switch(animALUOUT_step) 
      {
        case 0:
        {
          // Moving up
          deltaY = animALUOUT_0.y() - animALUOUT_1.y();
          animALUOUT_current = animALUOUT_0;
          animALUOUT_current.setY(animALUOUT_0.y() - deltaY / (NSTEPS3) * animCount);
          p.drawLine(animALUOUT_0, animALUOUT_current);
          break;
        }
        case 1:
        {        
          // Moving right
          deltaX = animALUOUT_2.x() - animALUOUT_1.x();
          animALUOUT_current = animALUOUT_1;
          animALUOUT_current.setX(animALUOUT_1.x() + deltaX / (NSTEPS3) * animCount);
          p.drawLine(animALUOUT_1, animALUOUT_current);
          break;
        }
        case 2:
        {
          // Moving up/down
          deltaY = animALUOUT_3.y() - animALUOUT_2.y();
          animALUOUT_current = animALUOUT_2;
          animALUOUT_current.setY(animALUOUT_2.y() + deltaY / (NSTEPS3) * animCount);
          p.drawLine(animALUOUT_2, animALUOUT_current);
          break;
        }
        case 3:
        {
          // Moving right
          deltaX = animALUOUT_4.x() - animALUOUT_3.x();
          animALUOUT_current = animALUOUT_3;
          animALUOUT_current.setX(animALUOUT_3.x() + deltaX / (NSTEPS3) * animCount);
          p.drawLine(animALUOUT_3, animALUOUT_current);
          break;
        }
      break;
      }
    p.drawText(animALUOUT_current,QString::fromStdString(OutString));
    break;
    }
    case END_ALUWRITE:
    {
      QRect stRect(animALUOUT_4, QSize(0,0));
      stRect = p.boundingRect(stRect, Qt::AlignCenter, OutString.c_str());
      p.drawText(stRect, Qt::AlignCenter, OutString.c_str());
      Redraw(p);
      cpu.regTargetObject->SetContents(writeContents, regTarget);
      stepType = START_ALUWRITE;
    }
    break;
    case START_ANIMATION3:
    {
      animCount = 0;
      anim3_step = 0;
      anim3_current = anim3_0;
      if(anim3DrawQueue.empty()) //If no more draw3s to be done
      {
        nextStepTime = d.msTime + SLOW_STEP;
        stepType = START_RNI;
        break;
      }
      else
      {
        pullAnim3DrawQueue(); //Setup the next anim3 animation from the queue
        stepType = ANIMATING3; //Do another animation3
      }
      // Fall through
    }
    case ANIMATING3:
    {
      Redraw(p);
      animCount++;
      if (animCount == NSTEPS3)
        {
          animCount = 0;
          anim3_step++;
        }
      if (anim3_step > 2)
        { // Done
          stepType = END_ANIMATION3;
          break;
        }
      if (anim3_step > 0) p.drawLine(anim3_0, anim3_1);
      if (anim3_step > 1) p.drawLine(anim3_1, anim3_2);
      switch(anim3_step)
      {
        case 0:
        {
          // Moving left
          deltaX = anim3_0.x() - anim3_1.x();
          anim3_current = anim3_0;
          anim3_current.setX(anim3_0.x() - deltaX / (NSTEPS3) * animCount);
          p.drawLine(anim3_0, anim3_current);
          break;
        }
        case 1:
        {
          // Moving down
          deltaY = anim3_2.y() - anim3_1.y();
          anim3_current = anim3_1;
          anim3_current.setY(anim3_1.y() + deltaY / (NSTEPS3) * animCount);
          p.drawLine(anim3_1, anim3_current);
          break;
        }
        case 2:
        {
          // Moving right
          deltaX = anim3_3.x() - anim3_2.x();
          anim3_current = anim3_2;
          anim3_current.setX(anim3_2.x() + deltaX / (NSTEPS3) * animCount);
          p.drawLine(anim3_2, anim3_current);
          break;
        }
      }
      break;
    }
    case END_ANIMATION3:
    {
      QRect stRect(anim3_current, QSize(0,0));
      stRect = p.boundingRect(stRect, Qt::AlignCenter, anim0String.c_str());
      p.drawText(stRect, Qt::AlignCenter, anim0String.c_str());
      Redraw(p);
      cpu.regTargetObject->SetContents(writeContents, regTarget);
      stepType = START_ANIMATION3;
      break;
    }
    case START_RNI:
    {
      cpu.pc += inst.addressAdder;
      inst.selectedMemLocation = (cpu.pc-inst.firstAddress)/inst.addressAdder; //We set the selected inst memory this way to account for jumps
      inst.updateRangeToShow();
      Redraw(p);
      stepType = RNI;
      break;
    }
    case RNI:
    {
      anim0 = cpu.pcMem.GetTopCenter(0);
      anim1 = inst.GetLockedLeftCenter(cpu.pc);
      anim2 = cpu.ciMem.GetTopCenter(0);
      readContents = inst.GetLocation(cpu.pc)->contents;
      stringstream ss;
      ss << cpu.pc;
      anim0String = ss.str();
      anim1String = readContents.GetString();
      stepType = START_ANIMATION;
      nextStepType = END_RNI;
      break;
    }
    case END_RNI:
    {
      cpu.ciMem.SetContents(readContents, 0);
      cpu.pcMem.SetContentsInt(cpu.pc, 0);
      // Now read, parse, and execute the instruction
      cpu.ExecuteNextInstruction();
      Redraw(p);
      if (cpu.halted) stepType = NONE;
      break;
    }
    case START_READ:
    {
      anim0 = cpu.ciMem.GetTopCenter(0);
      anim1 = data.GetRightCenter(readAddress);
      anim2 = cpu.regTargetObject->GetTopCenter(regTarget);
      stepType = START_ANIMATION;
      nextStepType = END_READ;
      break;
    }
    case END_READ:
    {
      // Set the register to the value
      cpu.regTargetObject->SetContents(readContents, regTarget);
      stepType = START_RNI;
      nextStepTime = d.msTime + SLOW_STEP;
      break;
    }
    case START_WRITE:
    {
      anim0 = cpu.ciMem.GetTopCenter(0);
      anim1 = data.GetRightCenter(writeAddress);
      anim2 = QPoint(); // No return, use START_WRITE2 instead
      stepType = START_ANIMATION;
      nextStepType = START_WRITE2;
      break;
    }
    case START_WRITE2:
    {
      anim0 = cpu.regSourceObject->GetTopCenter(regSource);
      anim1 = data.GetRightCenter(writeAddress);
      anim0String = anim1String;
      stepType = START_ANIMATION;
      nextStepType = END_WRITE;
      break;
    }
    case END_WRITE:
    {
      data.SetContents(writeContents, writeAddress);
      stepType = START_RNI;
      nextStepTime = d.msTime + SLOW_STEP;
      break;
    }
  }
  d.Update();
}

void Computer::Redraw(QPainter& p)
{
  QRect r(0, 0, d.width(), d.height());
  EraseRect(p, r);
  ComputerBox(p);
  data.Redraw(p);
  inst.Redraw(p);
  cpu.Redraw(p);
  myALU.Redraw(p);
  timeController.Redraw(p);
}

void Computer::ComputerBox(QPainter& p)
{
  // Draw a box around the whole thing
  QRect comp(8, 8, d.width()-16, d.height() - 16);
  QRect cTxt(8, 8, d.width() - 32, 16);
  QRect txt = p.boundingRect(cTxt, Qt::AlignCenter, "Computer");
  p.drawText(txt, Qt::AlignCenter, "Computer");
  comp.setY(comp.y() + txt.height());
  p.drawRect(comp);
}

void Computer::pushAnim3DrawQueue()
{
  Anim3Struct next;
  next.anim3           = anim3;
  next.anim3_0         = anim3_0;
  next.anim3_1         = anim3_1;
  next.anim3_2         = anim3_2;
  next.anim3_3         = anim3_3;
  next.anim0String     = anim0String;
  next.writeContents   = writeContents;
  next.regTargetObject = regTargetObject;
  next.regTarget       = regTarget;
  anim3DrawQueue.push(next);
}

void Computer::pushAnimALUDrawQueue()
{
  AnimALUStruct next;
  next.animALUINA_0           = animALUINA_0;
  next.animALUINA_1           = animALUINA_1;
  next.animALUINA_2           = animALUINA_2;
  next.animALUINA_3           = animALUINA_3;
  next.animALUINA_4           = animALUINA_4;
  next.animALUINB_0           = animALUINB_0;
  next.animALUINB_1           = animALUINB_1;
  next.animALUINB_2           = animALUINB_2;
  next.animALUINB_3           = animALUINB_3;
  next.animALUINB_4           = animALUINB_4;
  next.animALUOUT_0           = animALUOUT_0;
  next.animALUOUT_1           = animALUOUT_1;
  next.animALUOUT_2           = animALUOUT_2;
  next.animALUOUT_3           = animALUOUT_3;
  next.animALUOUT_4           = animALUOUT_4;
  next.AString                = AString;
  next.BString                = BString;
  next.OutString                = OutString;
  next.writeContents   = writeContents;
  next.regTargetObject = regTargetObject;
  next.regTarget       = regTarget;
  animALUDrawQueue.push(next);
}

void Computer::pullAnim3DrawQueue()
{
  Anim3Struct next = anim3DrawQueue.front();
  anim3           = next.anim3;
  anim3_0         = next.anim3_0;
  anim3_1         = next.anim3_1;
  anim3_2         = next.anim3_2;
  anim3_3         = next.anim3_3;
  anim0String     = next.anim0String;
  writeContents   = next.writeContents;
  regTargetObject = next.regTargetObject;
  regTarget       = next.regTarget;
  anim3DrawQueue.pop();
}

void Computer::pullAnimALUDrawQueue()
{
  AnimALUStruct next     = animALUDrawQueue.front();
  animALUINA_0           = next.animALUINA_0;
  animALUINA_1           = next.animALUINA_1;
  animALUINA_2           = next.animALUINA_2;
  animALUINA_3           = next.animALUINA_3;
  animALUINA_4           = next.animALUINA_4;
  animALUINB_0           = next.animALUINB_0;
  animALUINB_1           = next.animALUINB_1;
  animALUINB_2           = next.animALUINB_2;
  animALUINB_3           = next.animALUINB_3;
  animALUINB_4           = next.animALUINB_4;
  animALUOUT_0           = next.animALUOUT_0;
  animALUOUT_1           = next.animALUOUT_1;
  animALUOUT_2           = next.animALUOUT_2;
  animALUOUT_3           = next.animALUOUT_3;
  animALUOUT_4           = next.animALUOUT_4;
  AString                = next.AString;
  BString                = next.BString;
  OutString              = next.OutString;
  writeContents          = next.writeContents;
  regTargetObject        = next.regTargetObject;
  regTarget              = next.regTarget;
  animALUDrawQueue.pop();
}

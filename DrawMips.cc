// Draw a computer and component parts

#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>

#include "qdisplay.h"
#include "Computer.h"
#include "Memory.h"
#include "Instruction.h"

using namespace std;

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.connect(&app, SIGNAL(lastWindowClosed()), SLOT(quit()));
  QDisplay d(app);
  //d.BlankPixmap(512, 512);
  d.BlankPixmap(768, 768);
  d.Show();
  d.paused = true;
  d.paused = false;
  string memFile("mem1.txt");
  string instFile("inst1.txt");
  if (argc > 1) memFile = string(argv[1]);
  if (argc > 2) instFile = string(argv[2]);
  //Computer c(d, memFile.c_str(),instFile.c_str(), 8);
  Computer c(d, memFile.c_str(),instFile.c_str(), 32);
  //d->Pixmap()->save("computer.jpg");
  //exit(1);

  string extension("png");
  if (argc > 4) extension = string(argv[4]);
  if (argc > 3)
    {
      d.SaveName(argv[3], extension.c_str());
      //d.SaveTimerInterval(150); // Default is 10 fps, try 7.5
    }
  if (argc > 5) d.latexSaveFormat = true;
  
  // For debug testing animation
  c.stepType = Computer::RNI;
  c.nextStepType = Computer::END_RNI;
  //c.anim0 = c.cpu->cpuRect.topLeft();
  //c.anim1 = c.data->memory[0].contentsRect.bottomRight();
  //c.anim1.setY(c.anim1.y() - c.data->memory[0].contentsRect.height() / 2);
  //c.anim2 = c.cpu->regMem->memory[0].contentsRect.bottomLeft();
  c.nextStepTime = d.msTime + SLOW_STEP;
  d.StartMSTimer();
  while(!c.cpu.halted)
    {
      c.Step();
      app.processEvents();
    }
  d.WaitForMS(5000); // 5 seconds at end of sort for the movie
  d.SaveName(0, 0);
  app.exec();
}

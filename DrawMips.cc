// Draw a computer and component parts

#include <iostream>

#include <qpainter.h>
#include <qrect.h>
#include <qpoint.h>

#include "qdisplay.h"
#include "Computer.h"
#include "Memory.h"
#include "Instruction.h"

using namespace std;

#define WINDOW_WIDTH 768
#define WINDOW_HEIGHT 1024

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.connect(&app, SIGNAL(lastWindowClosed()), SLOT(quit()));
  QDisplay d(app);
  d.BlankPixmap(WINDOW_HEIGHT, WINDOW_WIDTH);
  d.Show();
  d.paused = true;
  d.paused = false;
	if (argc < 2)
	{
		cout << "No Arguments Passed. Exiting.\n";
		cout << "DrawMips Argument structure: ./DrawMips [ProgramName/REQUIRED] [FrameRate/1] [OutputMovieFile/OPTIONAL] [OutputMovieFileFormat/.png]";
		exit(1);
	}
	string input(argv[1]);
	int frameSkip = 1;
	if(argc > 2)
		frameSkip = atoi(argv[2]);
  Computer c(d, input.c_str(), 32, frameSkip);
  string extension("png");
	c.SLOW_STEP = 500;
	c.FAST_STEP = 200;
	c.QUICK_STEP = 0;
  if (argc > 3)
    {
      d.SaveName(argv[3], extension.c_str());
      //d.SaveTimerInterval(150); // Default is 10 fps, try 7.5
    }
  if (argc > 4) extension = string(argv[4]);
  if (argc > 5) d.latexSaveFormat = true;
  
  // For debug testing animation
  c.stepType = Computer::START_RNI;
  c.nextStepType = Computer::END_RNI;
  c.nextStepTime = d.msTime + c.SLOW_STEP;
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

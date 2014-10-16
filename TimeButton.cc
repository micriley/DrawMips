//  Handles the clickable buttons that deal with time
//  Michael K. Riley, Fall 2014

#include <qpainter.h>
#include <iostream>
#include <QImageReader>

#include "TimeButton.h"

TimeButton::TimeButton(const std::string& fileName,QPoint pos,QColor highlight,TimeButtonController& tbcr):topLeft(pos),highlightColor(highlight),selected(false),controller(tbcr)
{
  QString fileLocation(QString::fromStdString(fileName));
  if(!icon.load(fileLocation))
    {
    std::cout << fileLocation.toStdString() << " Didn't load!" << std::endl;
    QImageReader myImage(fileLocation);
    myImage.read(&icon);
    std::cout << myImage.error() << std::endl;
    std::cout << myImage.errorString().toStdString() << std::endl;
    }
}

QRect TimeButton::boundingRect()
{
  return QRect(topLeft,icon.size());
}

void TimeButton::draw(QPainter& p)
{
  Redraw(p);
}

void TimeButton::Redraw(QPainter& p)
{
  if(selected)
  {
    QImage coloredButton = icon;
    QPainter tempPaint(&coloredButton);
    tempPaint.setCompositionMode(QPainter::CompositionMode_SourceIn);
    tempPaint.fillRect(coloredButton.rect(), highlightColor);
    tempPaint.end();
    p.drawPixmap(topLeft,QPixmap::fromImage(coloredButton));
  }
  else
  {
    p.drawPixmap(topLeft,QPixmap::fromImage(icon));
  }
  return;
  //NOTE: got to get highlighting to work here
}

void TimeButton::mousePressEvent(QMouseEvent *event)
{
  selected = true;
  controller.buttonPressed(this); 
}

TimeButtonController::TimeButtonController(Computer* c0):c(c0),recordButton("Resources/Images/Record.png",QPoint(530,640),Qt::red,*this),pauseButton("Resources/Images/Pause.png",QPoint(600,570),Qt::yellow,*this),playButton("Resources/Images/Play.png",QPoint(530,570),Qt::green,*this),fastButton("Resources/Images/FastForward.png",QPoint(670,570),Qt::blue,*this),selectedButton(),currentState(PLAY)
{
  
}

void TimeButtonController::buttonPressed(TimeButton* newButton)
{
  selectedButton->selected = false;
  //Control code logic here
/*  if(newButton == pauseButton)
  {
    c.frameSkip = 0;
    currentState = PAUSE;
  }
  else if(newButton == fastButton)
  {
    if(currentState == PAUSE)
      c.frameSkip = 1;
    else
      c.frameSkip = c.frameSkip * 2;
    currentState = FAST;
  }
  else if(selectedButton == playButton)
  {
    if(currentState != PLAY)
    {
      c.frameSkip = 1;
      currentState = PLAY;
  }
  newButton.selected = true;
*/
  selectedButton = newButton;
}

void TimeButtonController::Redraw(QPainter& p)
{
  recordButton.Redraw(p);
  pauseButton.Redraw(p);
  playButton.Redraw(p);
  fastButton.Redraw(p);
}

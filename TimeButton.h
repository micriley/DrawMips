//Clickable run buttons. I call them Time Buttons
//Inherits QClickEvent and passes a state trasistion to the time-control code
//This way, buttons don't know how to control the time, the control code does it for them

#ifndef __TimeButton__
#define __TimeButton__

#include <QWidget>
#include <QEvent>
#include <QColor>
#include <QGraphicsEffect>

class TimeButtonController;
class Computer;

typedef enum
{
  PLAY,
  PAUSE,
  FAST,
  RECORD,
} TimeControlState_t;

class TimeButton : QWidget
{
Q_OBJECT;
  public:
    QImage icon;
    QPoint topLeft;
    QColor highlightColor;
    bool selected;
    TimeButtonController& controller;
  public:
    TimeButton(const std::string& fileName,QPoint pos,QColor highlight,TimeButtonController& tbcr);
    QRect boundingRect();
    void draw(QPainter& p);
    void Redraw(QPainter& p);
    void mousePressEvent(QMouseEvent *event);
};

class TimeButtonController //Needs to receive click events
{
  public:
    Computer* c;
    TimeButton recordButton;
    TimeButton pauseButton;
    TimeButton playButton;
    TimeButton fastButton;
    TimeButton* selectedButton;
    TimeControlState_t currentState;
  public:
    TimeButtonController(Computer* c0);
    void buttonPressed(TimeButton* newButton);
    void Redraw(QPainter& p);
};
static const std::string imagePath = ":/Resources/Images/";
#endif

#ifndef ACTION_H

#define ACTION_H

class Point;

class ActionManager
{
public:
    virtual void MouseMove(const Point &) = 0;
    virtual void PressButton(const Point &) = 0;
    virtual void ReleaseButton(const Point &) = 0;
    virtual void DoubleClick(const Point &) = 0;   
    virtual void KeyPress(char) = 0;   
	virtual void KeyRelease(char) = 0;   
    virtual void Exposed() = 0;
    virtual void OnQuit() = 0;
};

#endif


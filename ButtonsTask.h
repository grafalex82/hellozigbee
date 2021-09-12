#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

#include "PeriodicTask.h"
#include "Queue.h"
#include "AppQueue.h"

class ButtonsTask : public PeriodicTask
{
    uint32 pressedCounter;
    uint32 idleCounter;
    uint32 timeStamp;
    bool pressed;

public:
    ButtonsTask();

    static ButtonsTask * getInstance();

    bool handleDioInterrupt(uint32 dioStatus);
    bool canSleep() const;

protected:
    virtual void timerCallback();

private:
    void sendButtonEvent(ApplicationEventType evtType, uint8 button);
};

#endif // BUTTONSTASK_H

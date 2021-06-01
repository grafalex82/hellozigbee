#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

#include "PeriodicTask.h"
#include "Queue.h"

class ButtonsTask : public PeriodicTask
{
    uint32 pressedCounter;
    uint32 idleCounter;

public:
    ButtonsTask();

    static ButtonsTask * getInstance();

    bool handleDioInterrupt(uint32 dioStatus);
    bool canSleep() const;

protected:
    virtual void timerCallback();
};

#endif // BUTTONSTASK_H

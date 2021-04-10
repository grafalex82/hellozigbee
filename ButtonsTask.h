#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

#include "PeriodicTask.h"
#include "Queue.h"

class ButtonsTask : public PeriodicTask
{
    int pressedCounter;

public:
    ButtonsTask();

protected:
    virtual void timerCallback();
};

#endif // BUTTONSTASK_H

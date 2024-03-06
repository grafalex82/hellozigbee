#ifndef BLINKTASK_H
#define BLINKTASK_H

#include "PeriodicTask.h"
#include "GPIOPin.h"

extern "C"
{
#include "jendefs.h"
}

class BlinkTask : public PeriodicTask
{
    GPIOOutput ledPin;

public:
    BlinkTask();
    void init(uint32 mask);

    void setBlinkMode(bool fast);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

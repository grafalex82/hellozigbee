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
    void init(uint8 pin);

    void setBlinkMode(bool fast);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

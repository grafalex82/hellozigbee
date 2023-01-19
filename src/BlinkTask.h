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
    GPIOPin pin;

public:
    BlinkTask();
    void init(uint8 ledPin);

    void setBlinkMode(bool fast);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

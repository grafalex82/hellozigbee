#ifndef BLINKTASK_H
#define BLINKTASK_H

#include "PeriodicTask.h"

extern "C"
{
#include "jendefs.h"
}

class BlinkTask : public PeriodicTask
{
    bool fastBlinking;
    uint32 ledPinMask;

public:
    BlinkTask();
    void init(uint8 ledPin);

    void setBlinkMode(bool fast);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

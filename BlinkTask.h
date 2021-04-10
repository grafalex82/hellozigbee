#ifndef BLINKTASK_H
#define BLINKTASK_H

#include "PeriodicTask.h"

extern "C"
{
#include "jendefs.h"
}

class BlinkTask : public PeriodicTask
{
    bool_t * blinkMode;

public:
    BlinkTask(bool_t * modeVarPtr);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

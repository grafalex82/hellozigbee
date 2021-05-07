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

public:
    BlinkTask();

    void setBlinkMode(bool fast);

protected:
    virtual void timerCallback();
};

#endif // BLINKTASK_H

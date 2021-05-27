#ifndef SLEEPTIMER_H
#define SLEEPTIMER_H

#include <PeriodicTask.h>

class SleepTimer : public PeriodicTask
{
    uint32 cycleCounter;

public:
    SleepTimer();

    void reset();
    bool canSleep() const;

protected:
    virtual void timerCallback();
};

#endif // SLEEPTIMER_H

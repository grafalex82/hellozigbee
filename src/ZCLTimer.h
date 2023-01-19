#ifndef ZCLTIMER_H
#define ZCLTIMER_H

#include "PeriodicTask.h"

class ZCLTimer: public PeriodicTask
{
public:
    ZCLTimer();
    void init();

protected:
    virtual void timerCallback();
};

#endif // ZCLTIMER_H

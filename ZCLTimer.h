#ifndef ZCLTIMER_H
#define ZCLTIMER_H

#include "PeriodicTask.h"

class ZCLTimer: public PeriodicTask
{
public:
    ZCLTimer();

protected:
    virtual void timerCallback();
};

#endif // ZCLTIMER_H

#ifndef ZCLTIMER_H
#define ZCLTIMER_H

#include "PeriodicTask.h"

class ZCLTimer: public PeriodicTask
{
    uint32 tick1s;
    uint32 tick100ms;

    ZCLTimer();
public:
    static ZCLTimer * getInstance();
    void start();

protected:
    virtual void timerCallback();
};

#endif // ZCLTIMER_H

#ifndef PERIODIC_TASK_H
#define PERIODIC_TASK_H

#include "Timer.h"

extern "C"
{
#include "AppHardwareApi.h"
#include "dbg.h"
}

class PeriodicTask
{
protected:
    Timer timer;

public:
    void init()
    {
        timer.init(timerFunc, this);
    }

    void start(uint32 delay)
    {
        timer.start(delay);
    }

protected:
    static void timerFunc(void * param)
    {
        PeriodicTask * task = (PeriodicTask*)param;

        task->timerCallback();
    }

    virtual void timerCallback() = 0;
};

#endif //PERIODIC_TASK_H

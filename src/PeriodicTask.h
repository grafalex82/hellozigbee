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
    Timer timer;

public:
    void init()
    {
        timer.init(timerFunc, this);
    }

    void startTimer(uint32 delay)
    {
        timer.start(delay);
    }

    void stopTimer()
    {
        timer.stop();
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

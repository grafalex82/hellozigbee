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
    uint32 period;

public:
    void init(uint32 newPeriod = 0)
    {
        timer.init(timerFunc, this);
        setPeriod(newPeriod);
    }

    void setPeriod(uint32 newPeriod)
    {
        period = newPeriod;
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
        // Execute the task main work
        PeriodicTask * task = (PeriodicTask*)param;
        task->timerCallback();

        // Auto-reload timer
        if(task->period != 0)
            task->startTimer(task->period);
    }

    virtual void timerCallback() = 0;
};

#endif //PERIODIC_TASK_H

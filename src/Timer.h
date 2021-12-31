#ifndef TIMER_H
#define TIMER_H

extern "C"
{
#include "jendefs.h"
#include "ZTimer.h"
}

class Timer
{
    uint8 timerHandle;

public:
    void init(ZTIMER_tpfCallback cb, void * param, bool preventSleep = false)
    {
        ZTIMER_eOpen(&timerHandle, cb, param, preventSleep ? ZTIMER_FLAG_PREVENT_SLEEP : ZTIMER_FLAG_ALLOW_SLEEP);
    }

    void start(uint32 time)
    {
        ZTIMER_eStart(timerHandle, time);
    }

    void stop()
    {
        ZTIMER_eStop(timerHandle);
    }
};

#endif //TIMER_H


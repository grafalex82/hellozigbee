#include "ZCLTimer.h"

extern "C"
{
    #include "zcl.h"
    #include "dbg.h"
}

ZCLTimer::ZCLTimer()
{
    PeriodicTask::init(10);
    tick1s = 0;
    tick100ms = 0;
}

ZCLTimer * ZCLTimer::getInstance()
{
    static ZCLTimer instance;
    return &instance;
}

void ZCLTimer::start()
{
    startTimer(1000);   // Do not bother with timer events for the first second
}

void ZCLTimer::timerCallback()
{
    tick1s++;
    tick100ms++;

    if(tick100ms >= 10)
    {
        eZCL_Update100mS();

        tick100ms = 0;
    }

    if(tick1s >= 100)
    {
        // Process ZCL timers
        tsZCL_CallBackEvent sCallBackEvent;
        sCallBackEvent.pZPSevent = NULL;
        sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
        vZCL_EventHandler(&sCallBackEvent);

        tick1s = 0;
    }
}

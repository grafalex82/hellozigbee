#include "ZCLTimer.h"

extern "C"
{
    #include "zcl.h"
    #include "dbg.h"
}

ZCLTimer::ZCLTimer()
{
}

void ZCLTimer::init()
{
    PeriodicTask::init(10);
    tick1s = 0;
    tick100ms = 0;
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

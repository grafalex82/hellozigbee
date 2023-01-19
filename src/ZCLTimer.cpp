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
    PeriodicTask::init(1000);
}

void ZCLTimer::timerCallback()
{
    DBG_vPrintf(TRUE, "ZCLTimer::timerCallback(): Tick\n");

    // Process ZCL timers
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = NULL;
    sCallBackEvent.eEventType = E_ZCL_CBET_TIMER;
    vZCL_EventHandler(&sCallBackEvent);
}

#include "DeferredExecutor.h"

extern "C"
{
    #include "dbg.h"
    #include "ZTimer.h"
}

DeferredExecutor::DeferredExecutor()
{

}

void DeferredExecutor::init()
{
    delayQueue.init();
    callbacksQueue.init();
    paramsQueue.init();

    ZTIMER_eOpen(&timerHandle, timerCallback, this, ZTIMER_FLAG_ALLOW_SLEEP);
}

void DeferredExecutor::runLater(uint32 delay, deferredCallback cb, uint8 param)
{
    delayQueue.send(delay);
    callbacksQueue.send(cb);
    paramsQueue.send(param);

    scheduleNextCall();
}

void DeferredExecutor::timerCallback(void *timerParam)
{
    DBG_vPrintf(TRUE, "DeferredExecutor::timerCallback()\n");

    DeferredExecutor * executor = (DeferredExecutor *)timerParam;

    // Retrieve callback pointer and its parameter.
    deferredCallback cb;
    executor->callbacksQueue.receive(&cb);
    uint8 param;
    executor->paramsQueue.receive(&param);

    // Execute the callback
    (*cb)(param);

    // Get prepared for the next callback
    executor->scheduleNextCall();
}

void DeferredExecutor::scheduleNextCall()
{
    uint32 delay;
    if(ZTIMER_eGetState(timerHandle) != E_ZTIMER_STATE_RUNNING &&
       delayQueue.receive(&delay))
    {
        DBG_vPrintf(TRUE, "Scheduing next runLater call in %d ms\n", delay);

        ZTIMER_eStop(timerHandle);
        ZTIMER_eStart(timerHandle, delay);
    }

}

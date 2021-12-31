#include "BlinkTask.h"

extern "C"
{
#include "AppHardwareApi.h"
#include "dbg.h"
}

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly
BlinkTask::BlinkTask()
{
    fastBlinking = false;
    ledPinMask = 0;
}

void BlinkTask::init(uint8 ledPin)
{
    ledPinMask = 1UL << ledPin;
    vAHI_DioSetDirection(0, ledPinMask);

    PeriodicTask::init();
    startTimer(1000);
}

void BlinkTask::setBlinkMode(bool fast)
{
    fastBlinking = fast;
}

void BlinkTask::timerCallback()
{
    // toggle LED
    uint32 currentState = u32AHI_DioReadInput();
    vAHI_DioSetOutput(currentState ^ ledPinMask, currentState & ledPinMask);

    //Restart the timer
    startTimer(fastBlinking ? ZTIMER_TIME_MSEC(200) : ZTIMER_TIME_MSEC(1000));
}

#include "BlinkTask.h"

extern "C"
{
#include "AppHardwareApi.h"
#include "dbg.h"
}

static const uint32 FAST_BLINK_PERIOD = ZTIMER_TIME_MSEC(200);
static const uint32 SLOW_BLINK_PERIOD = ZTIMER_TIME_MSEC(1000);

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly
BlinkTask::BlinkTask()
{
    ledPinMask = 0;
}

void BlinkTask::init(uint8 ledPin)
{
    ledPinMask = 1UL << ledPin;
    vAHI_DioSetDirection(0, ledPinMask);

    PeriodicTask::init(SLOW_BLINK_PERIOD);
    startTimer(1000);
}

void BlinkTask::setBlinkMode(bool fast)
{
    setPeriod(fast ? FAST_BLINK_PERIOD : SLOW_BLINK_PERIOD);
}

void BlinkTask::timerCallback()
{
    // toggle LED
    uint32 currentState = u32AHI_DioReadInput();
    vAHI_DioSetOutput(currentState ^ ledPinMask, currentState & ledPinMask);
}

#include "BlinkTask.h"

#define BOARD_LED_BIT               (17)
#define BOARD_LED_PIN               (1UL << BOARD_LED_BIT)

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

    vAHI_DioSetDirection(0, BOARD_LED_PIN);

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
    vAHI_DioSetOutput(currentState^BOARD_LED_PIN, currentState&BOARD_LED_PIN);

    //Restart the timer
    startTimer(fastBlinking ? ZTIMER_TIME_MSEC(200) : ZTIMER_TIME_MSEC(1000));
}

#include "zcl_options.h"
#include "zps_gen.h"
#include "RelayTask.h"

RelayTask::RelayTask()
{
    PeriodicTask::init(50);

#ifdef RELAY1_ON_MASK
    ch1.init(RELAY1_ON_MASK, RELAY1_OFF_MASK);
#endif

#ifdef RELAY2_ON_MASK
    ch2.init(RELAY2_ON_MASK, RELAY2_OFF_MASK);
#endif
}

RelayTask * RelayTask::getInstance()
{
    static RelayTask instance;
    return &instance;
}

void RelayTask::setState(uint8 ep, bool on)
{
#ifdef RELAY1_ON_MASK
    if(ep == SWITCH1_ENDPOINT)
        ch1.setState(on);
#endif

#ifdef RELAY2_ON_MASK
    if(ep == SWITCH2_ENDPOINT)
        ch2.setState(on);
#endif

    startTimer(50);
}

void RelayTask::timerCallback()
{
    bool pulseInProgress = false;

#ifdef RELAY1_ON_MASK
    pulseInProgress |= ch1.update();
#endif
#ifdef RELAY2_ON_MASK
    pulseInProgress |= ch2.update();
#endif

    if(!pulseInProgress)
        stopTimer();
}

bool RelayTask::canSleep()
{
    return !isTimerActive();
}
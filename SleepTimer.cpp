#include "SleepTimer.h"

SleepTimer::SleepTimer()
{
    cycleCounter = 0;
    PeriodicTask::init();
    startTimer(1000);
}

void SleepTimer::reset()
{
    cycleCounter = 0;
}

bool SleepTimer::canSleep() const
{
    return cycleCounter > 50; // 50 * 100ms = 5s
}

void SleepTimer::timerCallback()
{
    cycleCounter++;
    startTimer(100);
}

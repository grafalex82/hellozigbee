extern "C"
{
    #include "portmacro.h"
    #include "zps_apl.h"
    #include "zps_apl_zdo.h"
    #include "dbg.h"
}

#include "PollTask.h"

PollTask::PollTask()
{
    pollPeriod = 0;
    PeriodicTask::init();
}

void PollTask::startPoll(int period)
{
    pollPeriod = period;
    startTimer(period);
}

void PollTask::stopPoll()
{
    stopTimer();
}

void PollTask::timerCallback()
{
    ZPS_eAplZdoPoll();

    // Restart the timer
    startTimer(pollPeriod);
}

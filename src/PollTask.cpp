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
    PeriodicTask::init(0);
}

void PollTask::startPoll(int period)
{
    setPeriod(period);
    startTimer(period);
}

void PollTask::stopPoll()
{
    stopTimer();
}

void PollTask::timerCallback()
{
    ZPS_eAplZdoPoll();
}

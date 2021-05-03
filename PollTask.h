#ifndef POLLTASK_H
#define POLLTASK_H

#include "PeriodicTask.h"

class PollTask : public PeriodicTask
{
    int pollPeriod;
    PollTask();

public:    
    static PollTask & getInstance();

    void startPoll(int period);
    void stopPoll();

protected:
    virtual void timerCallback();
};

#endif // POLLTASK_H

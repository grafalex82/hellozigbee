#ifndef RELAY_TASK_H
#define RELAY_TASK_H

#include "PeriodicTask.h"
#include "RelayHandler.h"

class RelayTask : public PeriodicTask
{
    RelayHandler ch1;
    RelayHandler ch2;

private:
    RelayTask();

public:
    static RelayTask * getInstance();

    void setState(uint8 ep, bool on);
    bool canSleep();

protected:
    virtual void timerCallback();    
};

#endif // RELAY_TASK_H

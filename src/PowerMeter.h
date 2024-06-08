#ifndef POWER_METER_H
#define POWER_METER_H

#include "GPIOPin.h"
#include "PeriodicTask.h"

class PowerMeter: public PeriodicTask
{
    GPIOOutput selPin;
    bool measureCurrent;
    PowerMeter();

public:
    static PowerMeter * getInstance();
    void start();

protected:
    virtual void timerCallback();
};

#endif // POWER_METER_H

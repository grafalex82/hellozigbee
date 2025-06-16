#ifndef POWER_METER_H
#define POWER_METER_H

#include "GPIOPin.h"
#include "PeriodicTask.h"

class IPowerMeasurementsConsumer;

class PowerMeter: public PeriodicTask
{
    GPIOOutput selPin;
    bool measureCurrent;
    IPowerMeasurementsConsumer * consumer;
    PowerMeter();

public:
    static PowerMeter * getInstance();
    void setConsumer(IPowerMeasurementsConsumer * cons);
    void start();

protected:
    virtual void timerCallback();
};

#endif // POWER_METER_H

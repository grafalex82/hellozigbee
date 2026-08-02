#ifndef ENERGY_METER_TASK_H
#define ENERGY_METER_TASK_H

#include "PeriodicTask.h"

// Acquires pulse frequencies from the on-board HLW8012 energy metering IC
// using the JN516x hardware pulse counters (no remapping needed):
// - CF  (active power pulses)                  -> DIO8 -> Pulse Counter 1
// - CF1 (current or voltage pulses, SEL-muxed) -> DIO1 -> Pulse Counter 0
// The counters run free; frequencies are derived from count deltas over the
// sampling period, with uint16 wrap-around arithmetic absorbing overflow.
class EnergyMeterTask : public PeriodicTask
{
    uint16 prevCfCount;
    uint16 prevCf1Count;
    uint16 prevTimebaseTicks;
    uint16 cfFreqDHz;   // last CF frequency in 0.1 Hz units
    uint16 cf1FreqDHz;  // last CF1 frequency in 0.1 Hz units
    uint32 cfTotal;     // cumulative CF pulses since boot (energy register)
    uint32 cf1Total;    // cumulative CF1 pulses since boot

private:
    EnergyMeterTask();

public:
    static EnergyMeterTask * getInstance();

    uint16 getCfFreqDHz() const { return cfFreqDHz; }
    uint16 getCf1FreqDHz() const { return cf1FreqDHz; }
    uint32 getCfTotal() const { return cfTotal; }
    uint32 getCf1Total() const { return cf1Total; }

protected:
    virtual void timerCallback();
};

#endif // ENERGY_METER_TASK_H

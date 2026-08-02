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
    uint16 cfFreqDHz;       // last CF frequency in 0.1 Hz units (active power)
    uint16 voltageFreqDHz;  // last CF1 frequency measured in voltage mode
    uint16 currentFreqDHz;  // last CF1 frequency measured in current mode
    uint32 cfTotal;         // cumulative CF pulses since boot (energy register)
    uint32 cf1Total;        // cumulative CF1 pulses since boot (mode-mixed, diagnostic)
    uint8 selCurrentMode;   // SEL state: 0 = voltage, 1 = current
    uint8 modeTicks;        // sampling windows spent in the present mode
    bool transitionWindow;  // window straddling a SEL toggle - not attributable

private:
    EnergyMeterTask();

public:
    static EnergyMeterTask * getInstance();

    uint16 getCfFreqDHz() const { return cfFreqDHz; }
    uint16 getVoltageFreqDHz() const { return voltageFreqDHz; }
    uint16 getCurrentFreqDHz() const { return currentFreqDHz; }
    uint32 getCfTotal() const { return cfTotal; }
    uint32 getCf1Total() const { return cf1Total; }

protected:
    virtual void timerCallback();
};

#endif // ENERGY_METER_TASK_H

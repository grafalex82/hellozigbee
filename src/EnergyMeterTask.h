#ifndef ENERGY_METER_TASK_H
#define ENERGY_METER_TASK_H

#include "PeriodicTask.h"
#include "PersistedValue.h"
#include "PdmIds.h"

class BasicClusterEndpoint;

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
    uint64 cfTotal;         // lifetime CF pulses (energy register, PDM-backed)
    uint32 cf1Total;        // cumulative CF1 pulses since boot (mode-mixed, diagnostic)
    uint8 selCurrentMode;   // SEL state: 0 = voltage, 1 = current
    uint8 modeTicks;        // sampling windows spent in the present mode
    bool transitionWindow;  // window straddling a SEL toggle - not attributable

    PersistedValue<uint64, PDM_ID_ENERGY> persistedEnergyPulses;
    uint64 lastSavedPulses;
    uint32 ticksSinceSave;

    BasicClusterEndpoint * meteringEndpoint;

private:
    EnergyMeterTask();

public:
    static EnergyMeterTask * getInstance();

    void setMeteringEndpoint(BasicClusterEndpoint * ep) { meteringEndpoint = ep; }

    // Raw acquisition values (calibration/diagnostic attributes)
    uint16 getCfFreqDHz() const { return cfFreqDHz; }
    uint64 getCfTotal() const { return cfTotal; }
    uint32 getCf1Total() const { return cf1Total; }

    // Calibrated electrical values (conversion constants live in the task)
    uint16 getActivePowerW() const;
    uint16 getVoltageDV() const;
    uint16 getCurrentMA() const;
    uint64 getEnergyWh() const;

protected:
    virtual void timerCallback();
};

#endif // ENERGY_METER_TASK_H

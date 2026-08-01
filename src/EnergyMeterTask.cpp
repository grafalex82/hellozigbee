#include "zcl_options.h"

#ifdef SUPPORTS_POWER_METERING

#include "EnergyMeterTask.h"

extern "C"
{
    #include "AppHardwareApi.h"
    #include "dbg.h"
}

// Must stay well below the ~30 s wrap time of a 16-bit counter at the
// maximum expected pulse frequency (~2.2 kHz on CF1 at 16 A)
static const uint32 SAMPLE_PERIOD_MS = 1000;

EnergyMeterTask::EnergyMeterTask()
{
    // Drive SEL low for a deterministic CF1 mode. The signal reaches the
    // HLW8012 inverted through a 2N7002; actual polarity is resolved later
    vAHI_DioSetDirection(0, METERING_SEL_MASK);
    vAHI_DioSetOutput(0, METERING_SEL_MASK);

    // CF/CF1 pins are the pulse counters' default inputs already, just make
    // sure they are inputs
    vAHI_DioSetDirection(METERING_CF_MASK | METERING_CF1_MASK, 0);

    // Rising edge, debounce off (debounce would cap counting at 1.2-3.7 kHz),
    // counters kept separate (both channels needed), no interrupts
    bAHI_PulseCounterConfigure(E_AHI_PC_1, 0, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_PulseCounterConfigure(E_AHI_PC_0, 0, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_StartPulseCounter(E_AHI_PC_1);
    bAHI_StartPulseCounter(E_AHI_PC_0);

    // Baseline the counts after start: bAHI_StartPulseCounter() may bump the
    // count by one even without a pulse
    bAHI_Read16BitCounter(E_AHI_PC_1, &prevCfCount);
    bAHI_Read16BitCounter(E_AHI_PC_0, &prevCf1Count);
    cfFreqDHz = 0;
    cf1FreqDHz = 0;

    PeriodicTask::init(SAMPLE_PERIOD_MS);
    startTimer(SAMPLE_PERIOD_MS);
}

EnergyMeterTask * EnergyMeterTask::getInstance()
{
    static EnergyMeterTask instance;
    return &instance;
}

void EnergyMeterTask::timerCallback()
{
    uint16 cfCount, cf1Count;
    bAHI_Read16BitCounter(E_AHI_PC_1, &cfCount);
    bAHI_Read16BitCounter(E_AHI_PC_0, &cf1Count);

    uint16 cfDelta = (uint16)(cfCount - prevCfCount);
    uint16 cf1Delta = (uint16)(cf1Count - prevCf1Count);
    prevCfCount = cfCount;
    prevCf1Count = cf1Count;

    // pulses over a 1 s window -> 0.1 Hz units, clamped against uint16 overflow
    cfFreqDHz = (cfDelta > 6553) ? 65535 : cfDelta * 10;
    cf1FreqDHz = (cf1Delta > 6553) ? 65535 : cf1Delta * 10;

    DBG_vPrintf(TRUE, "EnergyMeterTask: CF=%d.%d Hz, CF1=%d.%d Hz\n",
                cfFreqDHz / 10, cfFreqDHz % 10, cf1FreqDHz / 10, cf1FreqDHz % 10);
}

#endif // SUPPORTS_POWER_METERING

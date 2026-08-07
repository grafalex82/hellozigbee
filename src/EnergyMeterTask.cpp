#include "zcl_options.h"

#ifdef SUPPORTS_POWER_METERING

#include "EnergyMeterTask.h"
#include "BasicClusterEndpoint.h"

extern "C"
{
    #include "AppHardwareApi.h"
    #include "dbg.h"
}

// Must stay well below the ~30 s wrap time of a 16-bit counter at the
// maximum expected pulse frequency (~2.2 kHz on CF1 at 16 A)
static const uint32 SAMPLE_PERIOD_MS = 1000;

// Timer 0 free-runs as the sampling timebase: 16 MHz / 2^14 = 976.5625 Hz.
// The ZTIMER 1 s callback jitters when the main loop is busy (radio storms),
// so the window length is measured, never assumed. 16-bit wrap = 67 s.
static const uint8 TIMEBASE_PRESCALE = 14;
// freq[dHz] = pulses * 976.5625 * 10 / ticks = pulses * 78125 / (ticks * 8)
static const uint32 TIMEBASE_DHZ_MUL = 78125;
static const uint32 TIMEBASE_DHZ_DIV = 8;

// Conversion constants, scaled by 1e5: value = freq_dHz * K / 100000.
// Power: calibrated 2026-08-02 against an inline power meter (1910 W at
// 424.41 Hz CF over a 3 min pulse-count integration) -> 4.5004 W/Hz,
// +8.8 % over the datasheet-nominal 4.138 (shunt below its marked 2 mOhm).
// Voltage: calibrated against a multimeter at the load terminals under load
// (235.5 V read vs 225.3 V displayed -> +4.53 % over datasheet-nominal).
// Current: derived, not directly measured - all HLW8012 channels share Vref
// and the shunt, so Kc = Kp/Kv (+4.05 % over nominal; PF cross-check 0.966).
// Constants are specimen-calibrated on a QBKG11LM; QBKG11LM and QBKG12LM
// share the same board, so they serve as defaults for both.
static const uint32 METERING_W_PER_DHZ_E5 = 45004;
static const uint32 METERING_DV_PER_DHZ_E5 = 34176;
static const uint32 METERING_MA_PER_DHZ_E5 = 75357;

// SEL alternates CF1 between voltage and current mode every N sampling
// windows; the window straddling the toggle is discarded (mode change +
// HLW8012 settle), leaving N-1 valid windows per dwell
static const uint8 SEL_DWELL_TICKS = 5;

// Energy register persistence: save when this many pulses accumulated since
// the last save (~0.1 kWh at the calibrated 4.5 J/pulse), or daily if any
// unsaved energy exists. Keeps EEPROM wear negligible (>=5 years even at a
// continuous 5 kWh/day) while bounding the power-cut loss to ~0.1 kWh.
static const uint32 ENERGY_SAVE_PULSE_DELTA = 80000;
static const uint32 ENERGY_SAVE_MAX_TICKS = 86400;

EnergyMeterTask::EnergyMeterTask()
{
    // Drive SEL low for a deterministic CF1 mode. The signal reaches the
    // HLW8012 inverted through a 2N7002; actual polarity is resolved later
    vAHI_DioSetDirection(0, METERING_SEL_MASK);
    vAHI_DioSetOutput(0, METERING_SEL_MASK);

    // Rising edge, debounce off (debounce would cap counting at 1.2-3.7 kHz),
    // counters kept separate (both channels needed), no interrupts
    bAHI_PulseCounterConfigure(E_AHI_PC_1, 0, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_PulseCounterConfigure(E_AHI_PC_0, 0, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_StartPulseCounter(E_AHI_PC_1);
    bAHI_StartPulseCounter(E_AHI_PC_0);

    // Timebase timer: no interrupts, no output - and no DIO takeover, its
    // pins overlap CF (DIO8), SEL (DIO9) and the button (DIO10)
    vAHI_TimerDIOControl(E_AHI_TIMER_0, FALSE);
    vAHI_TimerEnable(E_AHI_TIMER_0, TIMEBASE_PRESCALE, FALSE, FALSE, FALSE);
    vAHI_TimerStartRepeat(E_AHI_TIMER_0, 0x0000, 0xFFFF);

    // Baseline the counts after start: bAHI_StartPulseCounter() may bump the
    // count by one even without a pulse
    bAHI_Read16BitCounter(E_AHI_PC_1, &prevCfCount);
    bAHI_Read16BitCounter(E_AHI_PC_0, &prevCf1Count);
    prevTimebaseTicks = u16AHI_TimerReadCount(E_AHI_TIMER_0);
    cfFreqDHz = 0;
    voltageFreqDHz = 0;
    currentFreqDHz = 0;
    cf1Total = 0;
    selCurrentMode = 0;         // constructor drove SEL low = voltage mode
    modeTicks = 0;
    transitionWindow = false;

    // Restore the lifetime energy register
    persistedEnergyPulses.init((uint64)0, "Energy");
    cfTotal = persistedEnergyPulses.getValue();
    lastSavedPulses = cfTotal;
    ticksSinceSave = 0;
    meteringEndpoint = NULL;

    PeriodicTask::init(SAMPLE_PERIOD_MS);
    startTimer(SAMPLE_PERIOD_MS);
}

EnergyMeterTask * EnergyMeterTask::getInstance()
{
    static EnergyMeterTask instance;
    return &instance;
}

static uint16 freqDHz(uint16 pulses, uint16 ticks)
{
    if(ticks == 0)
        return 0;

    uint64 f = (uint64)pulses * TIMEBASE_DHZ_MUL / ((uint32)ticks * TIMEBASE_DHZ_DIV);
    return (f > 65535) ? 65535 : (uint16)f;
}


uint16 EnergyMeterTask::getActivePowerW() const
{
    uint32 watts = (uint32)cfFreqDHz * METERING_W_PER_DHZ_E5 / 100000;
    return (watts > 32767) ? 32767 : watts;    // ZCL ActivePower is int16
}

uint16 EnergyMeterTask::getVoltageDV() const
{
    return (uint32)voltageFreqDHz * METERING_DV_PER_DHZ_E5 / 100000;
}

uint16 EnergyMeterTask::getCurrentMA() const
{
    uint32 mA = (uint32)currentFreqDHz * METERING_MA_PER_DHZ_E5 / 100000;
    return (mA > 65535) ? 65535 : mA;
}

uint64 EnergyMeterTask::getEnergyWh() const
{
    // Each CF pulse is a fixed energy quantum: Wh = pulses * (W/Hz) / 3600
    return cfTotal * METERING_W_PER_DHZ_E5 / 36000000;
}

void EnergyMeterTask::timerCallback()
{
    uint16 cfCount, cf1Count;
    bAHI_Read16BitCounter(E_AHI_PC_1, &cfCount);
    bAHI_Read16BitCounter(E_AHI_PC_0, &cf1Count);
    uint16 nowTicks = u16AHI_TimerReadCount(E_AHI_TIMER_0);

    uint16 cfDelta = (uint16)(cfCount - prevCfCount);
    uint16 cf1Delta = (uint16)(cf1Count - prevCf1Count);
    uint16 tickDelta = (uint16)(nowTicks - prevTimebaseTicks);
    prevCfCount = cfCount;
    prevCf1Count = cf1Count;
    prevTimebaseTicks = nowTicks;

    cfTotal += cfDelta;
    cf1Total += cf1Delta;

    cfFreqDHz = freqDHz(cfDelta, tickDelta);

    // Attribute the CF1 window to the mode that was active throughout it
    uint16 cf1FreqDHz = freqDHz(cf1Delta, tickDelta);
    if(transitionWindow)
        transitionWindow = false;
    else if(selCurrentMode)
        currentFreqDHz = cf1FreqDHz;
    else
        voltageFreqDHz = cf1FreqDHz;

    // Alternate SEL between voltage and current measurement
    if(++modeTicks >= SEL_DWELL_TICKS)
    {
        selCurrentMode ^= 1;
        // DIO9 low = voltage mode, high = current mode (2N7002 inverts on its way to SEL)
        if(selCurrentMode)
            vAHI_DioSetOutput(METERING_SEL_MASK, 0);
        else
            vAHI_DioSetOutput(0, METERING_SEL_MASK);
        modeTicks = 0;
        transitionWindow = true;
    }

    // Wear-aware persistence of the energy register
    ticksSinceSave++;
    if((cfTotal - lastSavedPulses >= ENERGY_SAVE_PULSE_DELTA) ||
       (cfTotal != lastSavedPulses && ticksSinceSave >= ENERGY_SAVE_MAX_TICKS))
    {
        persistedEnergyPulses.setValue(cfTotal);
        lastSavedPulses = cfTotal;
        ticksSinceSave = 0;
    }

    // Push fresh values into the ZCL cluster structs so both reads and the
    // attribute reporting engine (which samples the structs directly) see
    // current data
    if(meteringEndpoint)
        meteringEndpoint->updateMeteringAttributes();

    DBG_vPrintf(TRUE, "EnergyMeterTask: CF=%d.%d Hz, CF1=%d.%d Hz mode=%c (window %d ticks)\n",
                cfFreqDHz / 10, cfFreqDHz % 10, cf1FreqDHz / 10, cf1FreqDHz % 10,
                selCurrentMode ? 'I' : 'V', tickDelta);
}

#endif // SUPPORTS_POWER_METERING

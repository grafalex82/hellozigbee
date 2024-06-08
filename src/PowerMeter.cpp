#include "PowerMeter.h"

extern "C"
{
    #include "jendefs.h"
    #include "AppHardwareApi.h"
    #include "dbg.h"
    #include "zcl_options.h"
}

const uint32_t MEASUREMENT_PERIOD = 30000;
const float VREF = 2.43;
const uint32_t FOSC = 3579000;
const float R_SENSE = 0.002;

PowerMeter::PowerMeter()
{
    measureCurrent = false;
}

PowerMeter * PowerMeter::getInstance()
{
    static PowerMeter instance;
    return &instance;
}

void PowerMeter::start()
{
    DBG_vPrintf(TRUE, "PowerMeter::start\n");

    // Capture measurements every 15 seconds
    PeriodicTask::init(MEASUREMENT_PERIOD);
    startTimer(MEASUREMENT_PERIOD);

    // Select voltage measurement by setting SEL pin to 0 (the board has inverter on the SEL pin, so on the HLW8012 side it will be 1)
    selPin.init(HLW8012_SEL_MASK);
    selPin.setState(measureCurrent);

    // Enable pulse counters
    bAHI_PulseCounterConfigure(E_AHI_PC_0, FALSE, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_PulseCounterConfigure(E_AHI_PC_1, FALSE, 0, E_AHI_PC_COMBINE_OFF, FALSE);
    bAHI_Clear16BitPulseCounter(E_AHI_PC_0);
    bAHI_Clear16BitPulseCounter(E_AHI_PC_1);
    bAHI_StartPulseCounter(E_AHI_PC_0);
    bAHI_StartPulseCounter(E_AHI_PC_1);
}

void PowerMeter::timerCallback()
{
    // Read the pulse counter
    uint16_t pulseCounter0, pulseCounter1;
    bAHI_Read16BitCounter(E_AHI_PC_0, &pulseCounter0);
    bAHI_Clear16BitPulseCounter(E_AHI_PC_0);
    bAHI_Read16BitCounter(E_AHI_PC_1, &pulseCounter1);
    bAHI_Clear16BitPulseCounter(E_AHI_PC_1);
    DBG_vPrintf(TRUE, "Counter0: %d, Counter1: %d\n", pulseCounter0, pulseCounter1);

    // Common constants
    // float magicCoef = 1.085;
    // float voltageRatio = 1881.;

    if(measureCurrent)
    {
        // Calculate current
        float currentFreq = (float)pulseCounter0 / MEASUREMENT_PERIOD * 1000;
        //float currentCoef = VREF * 512 / (24 * FOSC) / R_SENSE; // 7.24 per pulse per mA    Xiaomi one: u16CurrentSlope = 765
        float current = currentFreq * 765;
        DBG_vPrintf(TRUE, "CurrentFreq: %d, Current: %d mA\n", (uint32_t)(currentFreq*1000), (uint32_t)(current*1000));
    }
    else
    {
        // Calculate voltage
        float voltageFreq = (float)pulseCounter0 / MEASUREMENT_PERIOD * 1000;
        //float voltageCoef = VREF * 512 / (2 * FOSC) * voltageRatio * magicCoef; // 0.3547   Xiaomi one: u16VoltageSlope = 35
        float voltage = voltageFreq * 0.3547;
        DBG_vPrintf(TRUE, "VoltageFreq: %d, Voltage: %d V\n", (uint32_t)(voltageFreq*1000), (uint32_t)voltage);
    }

    // Calculate power
    float powerFreq = (float)pulseCounter1 / MEASUREMENT_PERIOD * 1000;
    //float powerCoef = VREF * VREF * 128 / (48 * FOSC) * voltageRatio / R_SENSE * magicCoef; // 4.489    Xiaomi one: u16PowerSlope = 479
    float power = powerFreq * 4.489;
    DBG_vPrintf(TRUE, "PowerFreq: %d, Power: %d mW\n", (uint32_t)(powerFreq*1000), (uint32_t)(power * 1000.));
}




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

PowerMeter::PowerMeter()
{
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
    selPin.off();

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

    // Print values
    DBG_vPrintf(TRUE, "Counter0: %d, Counter1: %d\n", pulseCounter0, pulseCounter1);

    // Calculate voltage
    float voltageFreq = (float)pulseCounter0 / MEASUREMENT_PERIOD * 1000;
    float voltageRatio = 1881.;
    float magicCoef = 1.085;
    float voltage = voltageFreq * VREF * 512 / (2 * FOSC) * voltageRatio * magicCoef;
    float voltageCoef = VREF * 512 / (2 * FOSC) * voltageRatio * magicCoef; // 0.3547   Xiaomi one: u16VoltageSlope = 35
    DBG_vPrintf(TRUE, "VoltageFreq: %d, Voltage: %d V\n", (uint32_t)voltageFreq, (uint32_t)voltage);

    // Calculate power
    float powerFreq = (float)pulseCounter1 / MEASUREMENT_PERIOD * 1000;
    float Rsens = 0.002;
    float power = powerFreq * VREF * VREF * 128 / (48 * FOSC) * voltageRatio / Rsens;
    float powerCorrected = power * magicCoef;
    DBG_vPrintf(TRUE, "PowerFreq: %d, Power: %d mW, Power Corrected: %d mW\n", (uint32_t)powerFreq, (uint32_t)(power * 1000.), (uint32_t)(powerCorrected*1000.));
}




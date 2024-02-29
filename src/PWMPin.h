#ifndef PWMPIN_H
#define PWMPIN_H

extern "C"
{
#include "jendefs.h"
#include "AppHardwareApi.h"
}

const uint8 PWM_MAX = 255;

// Use the following table to map timerIDs to a pin number
// - E_AHI_TIMER_0 - 10 (Alternative pin - 4)
// - E_AHI_TIMER_1 - 11 (Alternative pin - 5)
// - E_AHI_TIMER_2 - 12 (Alternative pin - 6)
// - E_AHI_TIMER_3 - 13 (Alternative pin - 7)
// - E_AHI_TIMER_4 - 17 (Alternative pin - 8)
//
// To use alternative pin pass timerId | 0x80

class PWMPin
{
protected:
    uint8_t timerId;

public:
    PWMPin()
    {
        timerId = 0;
    }

    void init(uint8 timer)
    {
        timerId = timer & 0x7f;
        bool useAltPins = timer & 0x80;

        vAHI_TimerEnable(timerId, 6, FALSE, FALSE, TRUE);
        vAHI_TimerConfigureOutputs(timerId, TRUE, TRUE);

        if(useAltPins)
            vAHI_TimerSetLocation(timerId, TRUE, FALSE);

        setLevel(0);
    }

    void setLevel(uint8 level)
    {
        vAHI_TimerStartRepeat(timerId, (PWM_MAX - level), PWM_MAX);
    }
};

#endif // PWMPIN_H

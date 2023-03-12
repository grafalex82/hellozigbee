#ifndef PWMPIN_H
#define PWMPIN_H

extern "C"
{
#include "jendefs.h"
#include "AppHardwareApi.h"
}

const uint8 PWM_MAX = 255;

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
        timerId = timer;

        vAHI_TimerEnable(timerId, 6, FALSE, FALSE, TRUE);
        vAHI_TimerConfigureOutputs(timerId, TRUE, TRUE);
        setLevel(0);
    }

    void setLevel(uint8 level)
    {
        vAHI_TimerStartRepeat(timerId, (PWM_MAX - level), PWM_MAX);
    }
};

#endif // PWMPIN_H

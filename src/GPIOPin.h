#ifndef GPIOPIN_H
#define GPIOPIN_H

extern "C"
{
#include "jendefs.h"
#include "AppHardwareApi.h"
}

class GPIOPin
{
    uint32 pinMask;

public:
    GPIOPin()
    {
        pinMask = 0;
    }

    void init(uint8 pin)
    {
        pinMask = 1UL << pin;
        vAHI_DioSetDirection(0, pinMask);
    }

    void on()
    {
        vAHI_DioSetOutput(pinMask, 0);
    }

    void off()
    {
        vAHI_DioSetOutput(0, pinMask);
    }

    void toggle()
    {
        uint32 currentState = u32AHI_DioReadInput();
        vAHI_DioSetOutput(currentState ^ pinMask, currentState & pinMask);
    }
};


#endif // GPIOPIN_H

#ifndef GPIOPIN_H
#define GPIOPIN_H

extern "C"
{
#include "jendefs.h"
#include "AppHardwareApi.h"
}

class GPIOPin
{
protected:
    uint32 pinMask;

public:
    GPIOPin()
    {
        pinMask = 0;
    }

    void init(uint8 pin, bool output)
    {
        pinMask = 1UL << pin;

        if(output)
            vAHI_DioSetDirection(0, pinMask);
        else
            vAHI_DioSetDirection(pinMask, 0);
    }

    uint32 getPinMask() const
    {
        return pinMask;
    }
};

class GPIOOutput : public GPIOPin
{
public:
    void init(uint8 pin)
    {
        GPIOPin::init(pin, true);
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

class GPIOInput : public GPIOPin
{
public:
    void init(uint8 pin, bool pullUp)
    {
        GPIOPin::init(pin, false);
        vAHI_DioSetPullup(pullUp ? pinMask : 0, pullUp ? 0 : pinMask);
    }

    void enableInterrupt()
    {
        vAHI_DioInterruptEnable(pinMask, 0);
        vAHI_DioInterruptEdge(0, pinMask); //Falling edge interrupt (e.g. button is pressed and the pin is tied to the ground)
    }

    void enableWake()
    {
        vAHI_DioWakeEnable(pinMask, 0);
    }

    bool value() const
    {
        return (u32AHI_DioReadInput() & pinMask) != 0;
    }
};

#endif // GPIOPIN_H

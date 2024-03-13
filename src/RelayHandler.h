#ifndef RELAY_HANDLER_H
#define RELAY_HANDLER_H

extern "C"
{
    #include "jendefs.h"
}

#include "GPIOPin.h"

class RelayHandler
{
    GPIOOutput onPin;
    GPIOOutput offPin;
    uint8 remainingTicks;

public:
    RelayHandler();
    void init(uint32 onPinMask, uint32 offPinMask);
    void setState(bool state);

    bool update();
};

#endif // RELAY_HANDLER_H

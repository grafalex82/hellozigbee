#include "RelayHandler.h"

const uint8 PULSE_DURATION = 300 / 50; // 300 ms pulse duration / 50 ms per tick

RelayHandler::RelayHandler()
{
    remainingTicks = 0;
}

void RelayHandler::init(uint32 onPinMask, uint32 offPinMask)
{
    onPin.init(onPinMask);
    onPin.off();
    offPin.init(offPinMask);
    offPin.off();
    remainingTicks = 0;
}

void RelayHandler::setState(bool state)
{
    onPin.setState(state);
    offPin.setState(!state);

    remainingTicks = PULSE_DURATION;
}

bool RelayHandler::update()
{
    if(remainingTicks > 0)
    {
        remainingTicks--;
        if(remainingTicks == 0)
        {
            onPin.off();
            offPin.off();
        }
    }

    return remainingTicks > 0;    
}


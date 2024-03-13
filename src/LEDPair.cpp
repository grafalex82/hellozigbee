#include "LEDPair.h"
#include "ZigbeeDevice.h"

LEDPair::LEDPair()
{

}

void LEDPair::init(uint32 redPinMaskOrTimer, uint32 bluePinMaskOrTimer)
{
    red.init(redPinMaskOrTimer);
    blue.init(bluePinMaskOrTimer);
}

LEDHandler & LEDPair::active()
{
    if(ZigbeeDevice::getInstance()->isJoined())
        return blue;
    else
        return red;
}

LEDHandler & LEDPair::inactive()
{
    if(ZigbeeDevice::getInstance()->isJoined())
        return red;
    else
        return blue;
}

bool LEDPair::update()
{
    bool active = red.update();
    active |= blue.update();
    return active;
}

void LEDPair::setFixedLevel(uint8 level, uint8 step)
{
    active().setFixedLevel(level, step);
    inactive().setFixedLevel(0, step);
}

void LEDPair::startEffect(const LEDProgramEntry * effect)
{
    active().startEffect(effect);
    inactive().setFixedLevel(0);
}

void LEDPair::stopEffect()
{
    active().stopEffect();
    inactive().setFixedLevel(0);
}

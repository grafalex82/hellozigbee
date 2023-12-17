#include "LEDPair.h"
#include "ZigbeeDevice.h"

LEDPair::LEDPair()
{

}

void LEDPair::init(uint8 redTimer, uint8 blueTimer)
{
    red.init(redTimer);
    blue.init(blueTimer);
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

void LEDPair::update()
{
    red.update();
    blue.update();
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

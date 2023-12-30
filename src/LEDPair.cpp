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
    return ZigbeeDevice::getInstance()->isJoined() ? blue : red;
}

const LEDHandler & LEDPair::active() const
{
    return ZigbeeDevice::getInstance()->isJoined() ? blue : red;
}

LEDHandler & LEDPair::inactive()
{
    return ZigbeeDevice::getInstance()->isJoined() ? red : blue;
}

const LEDHandler & LEDPair::inactive() const
{
    return ZigbeeDevice::getInstance()->isJoined() ? red : blue;
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

uint8 LEDPair::getLevel() const
{
    return active().getLevel();
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

#include "LEDHandler.h"

LEDHandler::LEDHandler()
{

}

void LEDHandler::init(uint8 timer)
{
    pin.init(timer);
    curLevel = 0;
}

void LEDHandler::update()
{
    curLevel += 16;
    pin.setLevel(curLevel);
}
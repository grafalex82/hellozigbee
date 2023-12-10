#include "LEDHandler.h"

LEDHandler::LEDHandler()
{

}

void LEDHandler::init(uint8 timer)
{
    pin.init(timer);
    curLevel = 0;
    targetLevel = 0;
    increment = 0;

    incrementing = false;

    handlerState = STATE_FIXED_LEVEL;
}

void LEDHandler::handleStateIncrementing()
{
    int level = (int)curLevel + (int)increment;

    if(level > targetLevel)
        level = targetLevel;

    if(level > 255)
        level = 255;

    if(level == targetLevel)
        handlerState = STATE_FIXED_LEVEL;

    curLevel = (uint8)level;
    pin.setLevel(curLevel);
}

void LEDHandler::handleStateDecrementing()
{
    int level = (int)curLevel - (int)increment;

    if(level < 0)
        level = 0;

    if(level < targetLevel)
        level = targetLevel;

    if(level == targetLevel)
        handlerState = STATE_FIXED_LEVEL;

    curLevel = (uint8)level;
    pin.setLevel(curLevel);
}

void LEDHandler::update()
{
    switch(handlerState)
    {
        case STATE_INCREMENTING:
            handleStateIncrementing();
            break;

        case STATE_DECREMENTING:
            handleStateDecrementing();
            break;

        case STATE_FIXED_LEVEL:
        default:
            break;
    }

    // Temporary
    if(handlerState == STATE_FIXED_LEVEL)
    {
        incrementing = !incrementing;

        targetLevel = incrementing ? 255 : 0;
        increment = 4;
        handlerState = incrementing ? STATE_INCREMENTING : STATE_DECREMENTING;
    }
}
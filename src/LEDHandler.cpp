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
    inPause = false;

    handlerState = STATE_IDLE;

}

void LEDHandler::handleStateIncrementing()
{
    int level = (int)curLevel + (int)increment;

    if(level > targetLevel)
        level = targetLevel;

    if(level > 255)
        level = 255;

    if(level == targetLevel)
        handlerState = STATE_IDLE;

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
        handlerState = STATE_IDLE;

    curLevel = (uint8)level;
    pin.setLevel(curLevel);
}

void LEDHandler::handleStatePause()
{
    pauseCycles--;
    if(pauseCycles == 0)
        handlerState = STATE_IDLE;
}

void LEDHandler::handleStateMachine()
{
    switch(handlerState)
    {
        case STATE_INCREMENTING:
            handleStateIncrementing();
            break;

        case STATE_DECREMENTING:
            handleStateDecrementing();
            break;

        case STATE_PAUSE:
            handleStatePause();
            break;

        case STATE_IDLE:
        default:
            break;
    }
}

void LEDHandler::update()
{
    handleStateMachine();

    // if(handlerState == STATE_IDLE && programPtr != NULL)
    // {
    //     curProgramCmd = *programPtr;
    //     programPtr++;
    // }
    
    // Temporary
    if(handlerState == STATE_IDLE)
    {
        inPause = ~inPause;
        if(inPause)
        {
            pauseCycles = 50;
            handlerState = STATE_PAUSE;
        }
        else
        {
            incrementing = !incrementing;

            targetLevel = incrementing ? 255 : 0;
            increment = 5;
            handlerState = incrementing ? STATE_INCREMENTING : STATE_DECREMENTING;
        }
    }
}
#include "LEDHandler.h"

const LEDProgramEntry BLINK_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 64},     // Start with black
    {LED_CMD_PAUSE, 5, 0},              // Stay there for a 250 ms

    {LED_CMD_MOVE_TO_LEVEL, 255, 64},   // Blink fast to maximum, and then back to 0
    {LED_CMD_MOVE_TO_LEVEL, 0, 64},

    {LED_CMD_PAUSE, 5, 0},              // Stay for another 250 ms

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry BREATHE_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 200, 10},   // Gradually move to the bright level
    {LED_CMD_PAUSE, 5, 0},              // Stay there for a 250 ms
    {LED_CMD_MOVE_TO_LEVEL, 50, 10},    // Gradually move to the dimmed level
    {LED_CMD_PAUSE, 5, 0},              // Stay there for a 250 ms

    {LED_CMD_REPEAT, 4, 8},             // Jump 4 steps back, Repeat 8 times

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry OK_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 64},     // Start with black
    {LED_CMD_PAUSE, 5, 0},              // Stay there for a 250 ms

    {LED_CMD_MOVE_TO_LEVEL, 255, 80},   // Blink fast to maximum, and then back to 0
    {LED_CMD_MOVE_TO_LEVEL, 0, 80},
    {LED_CMD_MOVE_TO_LEVEL, 255, 80},   // Blink fast to maximum, and then back to 0
    {LED_CMD_MOVE_TO_LEVEL, 0, 80},

    {LED_CMD_PAUSE, 5, 0},              // Stay for another 250 ms

    {LED_CMD_STOP, 0, 0},
};


LEDHandler::LEDHandler()
{

}

void LEDHandler::init(uint8 timer)
{
    pin.init(timer);
    idleLevel = 0;
    curLevel = 0;
    targetLevel = 0;
    increment = 0;

    handlerState = STATE_IDLE;

    programPtr = OK_EFFECT;
    programIterations = 0;
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

void LEDHandler::moveToLevel(uint8 target, uint8 step)
{
    targetLevel = target;
    increment = step;
    handlerState = curLevel < targetLevel ? STATE_INCREMENTING : STATE_DECREMENTING;
}

void LEDHandler::pause(uint8 cycles)
{
    pauseCycles = cycles;
    handlerState = STATE_PAUSE;
}

void LEDHandler::handleProgramCommand()
{
    LEDProgramEntry command = *programPtr;
    programPtr++;

    switch(command.command)
    {
        // Schedule gradual movement to desired level (up or down)
        case LED_CMD_MOVE_TO_LEVEL:
            moveToLevel(command.param1, command.param2);
            break;

        // Schedule a short pause
        case LED_CMD_PAUSE:
            pause(command.param1);
            break;

        // Repeat few previous commands (number is in param1), until iterations counter matches param2
        case LED_CMD_REPEAT:
            programIterations++;

            if(programIterations >= command.param2)
                break;

            programPtr -= (command.param1 + 1);
            break;

        // Abandon program, transit to idle level
        case LED_CMD_STOP:  
            programPtr = NULL;
            moveToLevel(idleLevel, 10);
            break;

        default:
            break;
    }
}

void LEDHandler::update()
{
    handleStateMachine();

    if(handlerState == STATE_IDLE && programPtr != NULL)
        handleProgramCommand();
}
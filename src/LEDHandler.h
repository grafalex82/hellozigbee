#ifndef LEDHANDLER_H
#define LEDHANDLER_H

#include "PWMPin.h"

enum LEDProgramCommand
{
    LED_CMD_STOP,               // no params
    LED_CMD_MOVE_TO_LEVEL,      // param1 - target level, param2 - brightness increment per 50 ms
    LED_CMD_PAUSE,              // param1 - pause duration (in 50ms quants)
    LED_CMD_REPEAT              // param1 - target program index, param2 - number of iterations
};

struct LEDProgramEntry
{
    LEDProgramCommand command;
    uint8 param1;
    uint8 param2;
};


class LEDHandler
{
    PWMPin pin;
    uint8 curLevel;
    uint8 targetLevel;
    uint8 increment;
    uint8 pauseCycles;

    const LEDProgramEntry * programPtr;

    enum HandlerState
    {
        STATE_IDLE,
        STATE_INCREMENTING,
        STATE_DECREMENTING,
        STATE_PAUSE
    };

    HandlerState handlerState;

public:
    LEDHandler();
    void init(uint8 timer);
    void update();

protected:
    void handleStateMachine();
    void handleStateIncrementing();
    void handleStateDecrementing();
    void handleStatePause();

    void handleProgramCommand();
};  


#endif //LEDHANDLER_H
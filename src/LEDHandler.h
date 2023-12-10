#ifndef LEDHANDLER_H
#define LEDHANDLER_H

#include "PWMPin.h"

class LEDHandler
{
    PWMPin pin;
    uint8 curLevel;
    uint8 targetLevel;
    uint8 increment;

    // Temporary
    bool incrementing;

    enum ProgramState
    {
        IDLE,
        RUNNING
    };

    enum HandlerState
    {
        STATE_FIXED_LEVEL,
        STATE_INCREMENTING,
        STATE_DECREMENTING
    };

    HandlerState handlerState;

public:
    LEDHandler();
    void init(uint8 timer);
    void update();
};  


#endif //LEDHANDLER_H
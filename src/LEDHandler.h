#ifndef LEDHANDLER_H
#define LEDHANDLER_H

#include "PWMPin.h"

class LEDHandler
{
    PWMPin pin;
    uint8 curLevel;
    uint8 targetLevel;
    uint8 increment;
    uint8 pauseCycles;

    // Temporary
    bool incrementing;
    bool inPause;

    enum ProgramState
    {
        IDLE,
        RUNNING
    };

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
};  


#endif //LEDHANDLER_H
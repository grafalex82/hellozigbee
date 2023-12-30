#ifndef LEDHANDLER_H
#define LEDHANDLER_H

extern "C"
{
    #include "jendefs.h"
}

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


extern const LEDProgramEntry BLINK_EFFECT[];
extern const LEDProgramEntry BREATHE_EFFECT[];
extern const LEDProgramEntry OK_EFFECT[];
extern const LEDProgramEntry CHANNEL_CHANGE_EFFECT[];
extern const LEDProgramEntry NETWORK_SEARCH1_EFFECT[];
extern const LEDProgramEntry NETWORK_SEARCH2_EFFECT[];
extern const LEDProgramEntry NETWORK_CONNECT1_EFFECT[];
extern const LEDProgramEntry NETWORK_CONNECT2_EFFECT[];

class LEDHandler
{
    PWMPin pin;             // The Pin object where the LED is connected
    uint8 curLevel;         // Currently active brightness level (changes gradually on setting new brightness)
    uint8 targetLevel;      // Target brightness level while gradually increasing/decreasing brightness
    uint8 idleLevel;        // Selected brightness level when no effect active
    uint8 increment;        // Increment/decrement step when gradually changing brightness
    uint8 pauseCycles;      // Number of cycles to wait before switching to IDLE state

    const LEDProgramEntry * programPtr; // Pointer to the currently executed effect program, or NULL of no effect selected
    uint8 programIterations;            // Number of program iterations executed (used for LED_CMD_REPEAT command)

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

    void setFixedLevel(uint8 level, uint8 step = 10);
    uint8 getLevel() const;
    void startEffect(const LEDProgramEntry * effect);
    void stopEffect();

protected:
    void handleStateMachine();
    void handleStateIncrementing();
    void handleStateDecrementing();
    void handleStatePause();

    void handleProgramCommand();

    void setPWMLevel(uint8 level);
    void moveToLevel(uint8 target, uint8 step);
    void pause(uint8 cycles);
};  


#endif //LEDHANDLER_H
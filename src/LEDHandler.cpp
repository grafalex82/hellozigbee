#include "LEDHandler.h"

#ifdef SUPPORTS_PWM_LED
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

const LEDProgramEntry CHANNEL_CHANGE_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 255, 64},   // Maximum brightness for 0.5 sec
    {LED_CMD_PAUSE, 10, 0},
    {LED_CMD_MOVE_TO_LEVEL, 10, 80},    // The to minimum brightness for 7.5 seconds
    {LED_CMD_PAUSE, 150, 0},

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry NETWORK_CONNECT1_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Start with black, change instantly

    {LED_CMD_MOVE_TO_LEVEL, 255, 25},   // Blink medium fast
    {LED_CMD_MOVE_TO_LEVEL, 0, 25},

    {LED_CMD_REPEAT, 2, 25},            // Jump 2 steps back, Repeat 25 times (hopefully connect will happen earlier)

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry NETWORK_CONNECT2_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Start with white, change instantly

    {LED_CMD_MOVE_TO_LEVEL, 0, 25},     // Blink medium fast
    {LED_CMD_MOVE_TO_LEVEL, 255, 25},   

    {LED_CMD_REPEAT, 2, 25},            // Jump 2 steps back, Repeat 25 times (hopefully connect will happen earlier)

    {LED_CMD_STOP, 0, 0},
};

#else //SUPPORTS_PWM_LED

const LEDProgramEntry BLINK_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Start with black
    {LED_CMD_PAUSE, 10, 0},             // Stay there for a 500 ms

    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Blink fast to maximum for 0.5s, and then back to 0
    {LED_CMD_PAUSE, 10, 0},             
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},

    {LED_CMD_PAUSE, 10, 0},             // Stay for another 500 ms

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry BREATHE_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Turn on for 0.5s
    {LED_CMD_PAUSE, 10, 0},              
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Turn off for 0.5s
    {LED_CMD_PAUSE, 10, 0},              

    {LED_CMD_REPEAT, 4, 15},            // Jump 4 steps back, Repeat 15 times

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry OK_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Start with black
    {LED_CMD_PAUSE, 10, 0},             // Stay there for a 500 ms

    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Blink twice with 250 ms pause
    {LED_CMD_PAUSE, 5, 0},              
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},
    {LED_CMD_PAUSE, 5, 0},              
    {LED_CMD_MOVE_TO_LEVEL, 255, 255},
    {LED_CMD_PAUSE, 5, 0},              
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},

    {LED_CMD_PAUSE, 10, 0},              // Stay for another 500 ms

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry CHANNEL_CHANGE_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Start with black
    {LED_CMD_PAUSE, 5, 0},              // Stay there for a 250 ms

    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Turn on for 2s
    {LED_CMD_PAUSE, 40, 0},
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},    // Turn off for 3s
    {LED_CMD_PAUSE, 60, 0},

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry NETWORK_CONNECT1_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 255, 255},  // Blink fast with 0.2s period, starting ON state
    {LED_CMD_PAUSE, 2, 0},
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},
    {LED_CMD_PAUSE, 2, 0},

    {LED_CMD_REPEAT, 4, 50},            // Jump 4 steps back, Repeat 50 times (hopefully connect will happen earlier)

    {LED_CMD_STOP, 0, 0},
};

const LEDProgramEntry NETWORK_CONNECT2_EFFECT[] = 
{
    {LED_CMD_MOVE_TO_LEVEL, 0, 255},  // Blink fast with 0.2s period, starting OFF state
    {LED_CMD_PAUSE, 2, 0},
    {LED_CMD_MOVE_TO_LEVEL, 255, 255},
    {LED_CMD_PAUSE, 2, 0},

    {LED_CMD_REPEAT, 4, 50},            // Jump 2 steps back, Repeat 25 times (hopefully connect will happen earlier)

    {LED_CMD_STOP, 0, 0},
};

#endif //SUPPORTS_PWM_LED

LEDHandler::LEDHandler()
{

}

void LEDHandler::init(uint32 pinMaskOrTimer)
{
    pin.init(pinMaskOrTimer);
    idleLevel = 0;
    curLevel = 0;
    targetLevel = 0;
    increment = 0;

    handlerState = STATE_IDLE;
}

void LEDHandler::setPWMLevel(uint8 level)
{
#ifdef SUPPORTS_PWM_LED
    // Brightness to PWM level translation table for better perception of brightness change.
    // m = 253
    // p = 255
    // r = m*log10(2)/log10(p)
    // factor = 5
    // Yexp[level] = 2^((level-1)/r)-1                       // fixing logarithmic brightness perception
    // Y[level] = (Yexp[level] + factor*level)/(factor+1)    // mixing in some linear portion
    static const uint8 level2pwm[256] = {
        0x00, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x04,
        0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07,
        0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a,
        0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e,
        0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12,
        0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x16, 0x16, 0x16,
        0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1c,
        0x1c, 0x1d, 0x1d, 0x1d, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x20, 0x20, 0x21, 0x21, 0x21, 0x22, 0x22,
        0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x27, 0x28, 0x28, 0x29, 0x29, 0x2a, 0x2b,
        0x2b, 0x2c, 0x2c, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f, 0x30, 0x31, 0x31, 0x32, 0x33, 0x34, 0x34, 0x35,
        0x36, 0x37, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4d, 0x4e, 0x4f, 0x50, 0x51, 0x53, 0x54, 0x55,
        0x57, 0x58, 0x59, 0x5b, 0x5c, 0x5e, 0x5f, 0x61, 0x62, 0x64, 0x66, 0x67, 0x69, 0x6b, 0x6d, 0x6e,
        0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x83, 0x85, 0x87, 0x8a, 0x8c, 0x8e, 0x91,
        0x93, 0x96, 0x99, 0x9b, 0x9e, 0xa1, 0xa4, 0xa7, 0xaa, 0xad, 0xb0, 0xb3, 0xb7, 0xba, 0xbd, 0xc1,
        0xc4, 0xc8, 0xcc, 0xd0, 0xd3, 0xd7, 0xdb, 0xdf, 0xe4, 0xe8, 0xec, 0xf1, 0xf5, 0xfa, 0xff, 0xff
    };

    pin.setLevel(level2pwm[level]);
#else // SUPPORTS_PWM_LED
    pin.setState(level == 0);   // 0 - OFF, Non-0 - ON
#endif // SUPPORTS_PWM_LED
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
    setPWMLevel(curLevel);
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
    setPWMLevel(curLevel);
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

#ifdef SUPPORTS_PWM_LED
    increment = step;
#else
    increment = 255;    // Non-PWM LEDs are always switched instantly
#endif

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

bool LEDHandler::update()
{
    handleStateMachine();

    if(handlerState == STATE_IDLE && programPtr != NULL)
        handleProgramCommand();

    return handlerState != STATE_IDLE || programPtr != NULL;
}

void LEDHandler::setFixedLevel(uint8 level, uint8 step)
{
    programPtr = NULL;
    idleLevel = level;
    moveToLevel(level, step);
}

void LEDHandler::startEffect(const LEDProgramEntry * effect)
{
    programPtr = effect;

    if(programPtr)
    {
        // Set state to IDLE just to force switching to the program mode
        handlerState = STATE_IDLE;  

        // The new program will start from the very first iteration
        programIterations = 0;
    }
    else
        stopEffect();
}

void LEDHandler::stopEffect()
{
    // Abandon current program and get back to the previously set brightness level
    programPtr = NULL;
    moveToLevel(idleLevel, 10);
}

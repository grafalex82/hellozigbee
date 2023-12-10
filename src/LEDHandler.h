#ifndef LEDHANDLER_H
#define LEDHANDLER_H

#include "PWMPin.h"

class LEDHandler
{
    PWMPin pin;
    uint8 curLevel;

public:
    LEDHandler();
    void init(uint8 timer);
    void update();
};  


#endif //LEDHANDLER_H
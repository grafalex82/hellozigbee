#ifndef LEDPAIR_H
#define LEDPAIR_H

#include "LEDHandler.h"

class LEDPair
{
    LEDHandler red;
    LEDHandler blue;

    LEDHandler & active();
    LEDHandler & inactive();

public:
    LEDPair();
    void init(uint8 redTimer, uint8 blueTimer);

    void update();

    void setFixedLevel(uint8 level, uint8 step = 10);
    void startEffect(const LEDProgramEntry * effect);
    void stopEffect();
};

#endif // LEDPAIR_H
#ifndef LEDPAIR_H
#define LEDPAIR_H

#include "LEDHandler.h"

class LEDPair
{
    LEDHandler red;
    LEDHandler blue;

    LEDHandler & active();
    const LEDHandler & active() const;
    LEDHandler & inactive();
    const LEDHandler & inactive() const;

public:
    LEDPair();
    void init(uint8 redTimer, uint8 blueTimer);

    void update();

    void setFixedLevel(uint8 level, uint8 step = 10);
    uint8 getLevel() const;
    void startEffect(const LEDProgramEntry * effect);
    void stopEffect();
};

#endif // LEDPAIR_H
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
    void init(uint32 redPinMaskOrTimer, uint32 bluePinMaskOrTimer);

    bool update();

    void setFixedLevel(uint8 level, uint8 step = 10);
    void startEffect(const LEDProgramEntry * effect);
    void stopEffect();
};

#endif // LEDPAIR_H
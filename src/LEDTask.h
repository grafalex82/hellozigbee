#ifndef LEDTASK_H
#define LEDTASK_H

#include "PeriodicTask.h"
#include "LEDHandler.h"

extern "C"
{
    #include "zcl.h"
    #include "zcl_options.h"
}

enum LEDCommand
{
    CHANNEL_OFF,
    CHANNEL_ON,
    CHANNEL_SET_LEVEL,
    EFFECT_BLINK,
    EFFECT_BREATHE,
    EFFECT_OK,
    EFFECT_STOP
};



class LEDTask : public PeriodicTask
{
    LEDHandler led1red;
    //LEDHandler led1blue;
    LEDHandler led2red;
    //LEDHandler led2blue;

    enum State
    {
        IDLE,
        MOVE_TO
    };

    unsigned char currentValue;
    unsigned char targetValue;
    char increment;

private:
    LEDTask();

public:
    static LEDTask * getInstance();
    void start();

    void stopEffect();
    void triggerEffect(uint8 effect);
    void triggerSpecialEffect(uint8 effect);

protected:
    virtual void timerCallback();    
};

#endif //LEDTASK_H

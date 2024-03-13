#ifndef LEDTASK_H
#define LEDTASK_H

#include "PeriodicTask.h"
#include "LEDHandler.h"
#include "LEDPair.h"

extern "C"
{
    #include "zcl.h"
    #include "zcl_options.h"
}

enum LEDTaskSpecialEffect
{
    LED_TASK_NETWORK_CONNECT_EFFECT
    // Other effects TBD
};

class LEDTask : public PeriodicTask
{
    LEDPair ch1;

#ifdef LED2_RED_MASK_OR_TIMER
    LEDPair ch2;
#endif

private:
    LEDTask();

public:
    static LEDTask * getInstance();

    void stopEffect();
    void setFixedLevel(uint8 ep, uint8 level);
    void triggerEffect(uint8 ep, uint8 effect);
    void triggerSpecialEffect(LEDTaskSpecialEffect effect);

    bool canSleep();

protected:
    void activate();
    void deactivate();
    virtual void timerCallback();    
};

#endif //LEDTASK_H

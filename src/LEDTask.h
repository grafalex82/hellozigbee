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
    LEDPair ch2;

private:
    LEDTask();

public:
    static LEDTask * getInstance();
    void start();

    void stopEffect();
    void setFixedLevel(uint8 ep, uint8 level, uint8 step = 10);
    uint8 getLevel(uint8 ep) const;
    void triggerEffect(uint8 ep, uint8 effect);
    void triggerSpecialEffect(LEDTaskSpecialEffect effect);

protected:
    virtual void timerCallback();    
};

#endif //LEDTASK_H

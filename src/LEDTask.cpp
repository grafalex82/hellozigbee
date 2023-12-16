extern "C"
{
    #include "jendefs.h"
    #include "zps_gen.h"
    #include "dbg.h"
}

#include "LEDTask.h"


// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly
LEDTask::LEDTask()
{
    PeriodicTask::init(50);

    led1red.init(E_AHI_TIMER_3);
    //led1blue.init(E_AHI_TIMER_4);
    led2red.init(E_AHI_TIMER_1);
    //led2blue.init(E_AHI_TIMER_2);
}

LEDTask * LEDTask::getInstance()
{
    static LEDTask instance;
    return &instance;
}

void LEDTask::start()
{
    stopEffect();
    startTimer(50);
}

void LEDTask::stopEffect()
{
    led1red.stopEffect();
    led2red.stopEffect();
}

void LEDTask::setFixedLevel(uint8 ep, uint8 level)
{
    if(ep == HELLOZIGBEE_SWITCH1_ENDPOINT)
    {
        led1red.setFixedLevel(level);
        led2red.stopEffect();
    }

    if(ep == HELLOZIGBEE_SWITCH2_ENDPOINT)
    {
        led1red.stopEffect();
        led2red.setFixedLevel(level);
    }
}

void LEDTask::triggerEffect(uint8 ep, uint8 effect)
{
    bool ch1 = ep != HELLOZIGBEE_SWITCH2_ENDPOINT;
    bool ch2 = ep != HELLOZIGBEE_SWITCH1_ENDPOINT;

    const LEDProgramEntry * program = NULL;
    if(effect == 0)
        program = BLINK_EFFECT;
    if(effect == 1)
        program = BREATHE_EFFECT;
    if(effect == 2)
        program = OK_EFFECT;
    if(effect == 11)
        program = CHANNEL_CHANGE_EFFECT;

    if(ch1)
        led1red.startEffect(program);
    if(ch2)
        led2red.startEffect(program);
}

void LEDTask::triggerSpecialEffect(uint8 effect)
{
    const LEDProgramEntry * program1 = NULL;
    const LEDProgramEntry * program2 = NULL;

    // Network search effect
    if(effect == 0)
    {
        program1 = NETWORK_SEARCH1_EFFECT;
        program2 = NETWORK_SEARCH2_EFFECT;
    }

    // Connecting effect
    if(effect == 1)
    {
        program1 = NETWORK_CONNECT1_EFFECT;
        program2 = NETWORK_CONNECT2_EFFECT;
    }

    led1red.startEffect(program1);
    led2red.startEffect(program2);

}

void LEDTask::timerCallback()
{
    led1red.update();
    led2red.update();
}

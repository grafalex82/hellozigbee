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

    ch1.init(LED1_RED_TIMER, LED1_BLUE_TIMER);
    // ch2.init(LED2_RED_TIMER, LED2_BLUE_TIMER);
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
    ch1.stopEffect();
    // ch2.stopEffect();
}

void LEDTask::setFixedLevel(uint8 ep, uint8 level)
{
    if(ep == SWITCH1_ENDPOINT)
        ch1.setFixedLevel(level);
    // if(ep == HELLOZIGBEE_SWITCH2_ENDPOINT)
    //     ch2.setFixedLevel(level);
}

void LEDTask::triggerEffect(uint8 ep, uint8 effect)
{
    //bool setCh1 = ep != HELLOZIGBEE_SWITCH2_ENDPOINT;
    // bool setCh2 = ep != HELLOZIGBEE_SWITCH1_ENDPOINT;

    const LEDProgramEntry * program = NULL;
    if(effect == 0)
        program = BLINK_EFFECT;
    if(effect == 1)
        program = BREATHE_EFFECT;
    if(effect == 2)
        program = OK_EFFECT;
    if(effect == 11)
        program = CHANNEL_CHANGE_EFFECT;

    //if(setCh1)
        ch1.startEffect(program);
    // if(setCh2)
    //     ch2.startEffect(program);
}

void LEDTask::triggerSpecialEffect(LEDTaskSpecialEffect effect)
{
    const LEDProgramEntry * program1 = NULL;
    // const LEDProgramEntry * program2 = NULL;

    // Network joining effect
    if(effect == LED_TASK_NETWORK_CONNECT_EFFECT)
    {
        program1 = NETWORK_CONNECT1_EFFECT;
//        program2 = NETWORK_CONNECT2_EFFECT;
    }

    // Other effects TBD

    // Apply the effect
    ch1.startEffect(program1);
    // ch2.startEffect(program2);
}

void LEDTask::timerCallback()
{
    ch1.update();
    // ch2.update();
}

extern "C"
{
    #include "jendefs.h"
    #include "zps_gen.h"
    #include "dbg.h"
}

#include "LEDTask.h"
#include "zcl_options.h"

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly
LEDTask::LEDTask()
{
    PeriodicTask::init(50);

    ch1.init(LED1_RED_MASK_OR_TIMER, LED1_BLUE_MASK_OR_TIMER);
#ifdef LED2_RED_MASK_OR_TIMER
    ch2.init(LED2_RED_MASK_OR_TIMER, LED2_BLUE_MASK_OR_TIMER);
#endif

    stopEffect();
    deactivate();
}

LEDTask * LEDTask::getInstance()
{
    static LEDTask instance;
    return &instance;
}

void LEDTask::stopEffect()
{
    ch1.stopEffect();

#ifdef LED2_RED_MASK_OR_TIMER
    ch2.stopEffect();
#endif
}

void LEDTask::setFixedLevel(uint8 ep, uint8 level)
{
    if(ep == SWITCH1_ENDPOINT)
        ch1.setFixedLevel(level);

#ifdef LED2_RED_MASK_OR_TIMER
    if(ep == SWITCH2_ENDPOINT)
        ch2.setFixedLevel(level);
#endif

    activate();
}

void LEDTask::triggerEffect(uint8 ep, uint8 effect)
{
    const LEDProgramEntry * program = NULL;
    if(effect == 0)
        program = BLINK_EFFECT;
    if(effect == 1)
        program = BREATHE_EFFECT;
    if(effect == 2)
        program = OK_EFFECT;
    if(effect == 11)
        program = CHANNEL_CHANGE_EFFECT;

    if(ep == BASIC_ENDPOINT || ep == SWITCH1_ENDPOINT)
        ch1.startEffect(program);

#ifdef LED2_RED_MASK_OR_TIMER
    if(ep == BASIC_ENDPOINT || ep == SWITCH2_ENDPOINT)
        ch2.startEffect(program);
#endif

    activate();
}

void LEDTask::triggerSpecialEffect(LEDTaskSpecialEffect effect)
{
    const LEDProgramEntry * program1 = NULL;
#ifdef LED2_RED_MASK_OR_TIMER
    const LEDProgramEntry * program2 = NULL;
#endif

    // Network joining effect
    if(effect == LED_TASK_NETWORK_CONNECT_EFFECT)
    {
        program1 = NETWORK_CONNECT1_EFFECT;
#ifdef LED2_RED_MASK_OR_TIMER
        program2 = NETWORK_CONNECT2_EFFECT;
#endif
    }

    // Other effects TBD

    // Apply the effect
    ch1.startEffect(program1);

#ifdef LED2_RED_MASK_OR_TIMER
    ch2.startEffect(program2);
#endif

    activate();
}

void LEDTask::timerCallback()
{
    bool active = ch1.update();

#ifdef LED2_RED_MASK_OR_TIMER
    active |= ch2.update();
#endif

    if(!active)
        deactivate();
}

void LEDTask::activate()
{
    startTimer(50);
}

void LEDTask::deactivate()
{
    stopTimer();
}

bool LEDTask::canSleep()
{
    return !isTimerActive();
}
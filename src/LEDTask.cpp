#include "LEDTask.h"

extern "C"
{
    #include "dbg.h"
}


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
    startTimer(50);
}

void LEDTask::timerCallback()
{
    led1red.update();
    led2red.update();
}

#include "ButtonsTask.h"
#include "AppQueue.h"

#define BOARD_BTN_BIT               (1)
#define BOARD_BTN_PIN               (1UL << BOARD_BTN_BIT)

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly

ButtonsTask::ButtonsTask()
{
    pressedCounter = 0;
    idleCounter = 0;
    timeStamp = 0;
    pressed = false;

    // Set up GPIO for the button
    vAHI_DioSetDirection(BOARD_BTN_PIN, 0);
    vAHI_DioSetPullup(BOARD_BTN_PIN, 0);
    vAHI_DioInterruptEdge(0, BOARD_BTN_PIN);
    vAHI_DioWakeEnable(BOARD_BTN_PIN, 0);

    PeriodicTask::init();
    startTimer(1000);
}

ButtonsTask * ButtonsTask::getInstance()
{
    static ButtonsTask instance;
    return &instance;
}

bool ButtonsTask::handleDioInterrupt(uint32 dioStatus)
{
    if(dioStatus & BOARD_BTN_PIN)
    {
        idleCounter = 0;
        return true;
    }

    return false;
}

bool ButtonsTask::canSleep() const
{
    return idleCounter > 500; // 500 cycles * 10 ms = 5 sec
}

void ButtonsTask::timerCallback()
{
    timeStamp++;

    uint32 input = u32AHI_DioReadInput();
    bool btnState = (input & BOARD_BTN_PIN) == 0;

    if(btnState)
    {
        idleCounter = 0;
        pressedCounter++;
        //DBG_vPrintf(TRUE, "Button still pressed for %d ticks\n", pressedCounter);

        if(!pressed && (pressedCounter >= 2)) // Just pressed?
        {
            pressed = true;
            DBG_vPrintf(TRUE, "Detected button press\n");
            ApplicationEvent evt = {BUTTON_PRESS, 0, timeStamp};
            appEventQueue.send(evt);
        }
    }
    else if(pressed)
    {
        // detect very long press
        if(pressedCounter > 500) // 5 sec
        {
            DBG_vPrintf(TRUE, "Button released. Long press detected\n");
            ApplicationEvent evt = {VERY_LONG_PRESS, 0, timeStamp};
            appEventQueue.send(evt);
        }

        // detect short press
        else
        {
            DBG_vPrintf(TRUE, "Button released. Short press detected\n");
            ApplicationEvent evt = {BUTTON_RELEASE, 0, timeStamp};
            appEventQueue.send(evt);
        }

        pressed = false;
        pressedCounter = 0;
        idleCounter++;
    }

    startTimer(10);
}



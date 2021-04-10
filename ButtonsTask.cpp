#include "ButtonsTask.h"
#include "AppQueue.h"

#define BOARD_BTN_BIT               (1)
#define BOARD_BTN_PIN               (1UL << BOARD_BTN_BIT)

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly

ButtonsTask::ButtonsTask()
{
    // Set up GPIO for the button
    vAHI_DioSetDirection(BOARD_BTN_PIN, 0);
    vAHI_DioSetPullup(BOARD_BTN_PIN, 0);

    PeriodicTask::init();
    startTimer(1000);
}

void ButtonsTask::timerCallback()
{
    uint32 input = u32AHI_DioReadInput();
    bool btnState = (input & BOARD_BTN_PIN) == 0;

    if(btnState)
    {
        pressedCounter++;
        DBG_vPrintf(TRUE, "Button still pressed for %d ticks\n", pressedCounter);
    }
    else
    {
        // detect long press
        if(pressedCounter > 200)
        {
            DBG_vPrintf(TRUE, "Button released. Long press detected\n");
            appEventQueue.send(BUTTON_LONG_PRESS);
        }

        // detect short press
        else if(pressedCounter > 5)
        {
            DBG_vPrintf(TRUE, "Button released. Short press detected\n");
            appEventQueue.send(BUTTON_SHORT_PRESS);
        }

        pressedCounter = 0;
    }

    startTimer(10);
}


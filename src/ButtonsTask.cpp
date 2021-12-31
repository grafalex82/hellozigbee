#include "ButtonsTask.h"
#include "IButtonHandler.h"
#include "AppQueue.h"


// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly

ButtonsTask::ButtonsTask()
{
    idleCounter = 0;
    longPressCounter = 0;

    buttonsMask = 0;

    numHandlers = 0;

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
    if(dioStatus & buttonsMask)
    {
        idleCounter = 0;
        return true;
    }

    return false;
}

bool ButtonsTask::canSleep() const
{
    return idleCounter > 5000 / ButtonPollCycle; // 500 cycles * 10 ms = 5 sec
}

void ButtonsTask::registerHandler(uint32 pinMask, IButtonHandler * handler)
{
    DBG_vPrintf(TRUE, "ButtonsTask::registerHandler(): Registering a handler for mask=%08x\n", pinMask);

    // Store the handler pointer
    handlers[numHandlers].pinMask = pinMask;
    handlers[numHandlers].handler = handler;
    numHandlers++;

    // Update the pin mask for all buttons
    buttonsMask |= pinMask;

    // Set up GPIO for the button
    vAHI_DioSetDirection(pinMask, 0);
    vAHI_DioSetPullup(pinMask, 0);
    vAHI_DioInterruptEdge(0, pinMask);
    vAHI_DioWakeEnable(pinMask, 0);
}

void ButtonsTask::timerCallback()
{
    uint32 input = u32AHI_DioReadInput();
    bool someButtonPressed = false;

    //DBG_vPrintf(TRUE, "ButtonsTask::timerCallback(): input=%08x\n", input);
    for(uint8 h = 0; h < numHandlers; h++)
    {
        bool pressed = ((input & handlers[h].pinMask) == 0);
        //DBG_vPrintf(TRUE, "ButtonsTask::timerCallback(): handler pinMask=%08x (pressed=%d)\n", handlers[h].pinMask, pressed);
        handlers[h].handler->handleButtonState(pressed);

        if(pressed)
            someButtonPressed = true;
    }

    // Reset the idle counter when user interacts with a button
    if(someButtonPressed)
    {
        idleCounter = 0;
        longPressCounter++;
    }
    else
    {
        idleCounter++;
        longPressCounter = 0;
    }

    // Process a very long press to join/leave the network
    if(longPressCounter > 5000/ButtonPollCycle)
    {
        ApplicationEvent evt = {BUTTON_VERY_LONG_PRESS, 0};
        appEventQueue.send(evt);

        for(uint8 h = 0; h < numHandlers; h++)
            handlers[h].handler->resetButtonStateMachine();

        longPressCounter = 0;
    }

    startTimer(ButtonPollCycle);
}



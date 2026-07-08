#include "ZigbeeDevice.h"
#include "ButtonsTask.h"
#include "IButtonHandler.h"


// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly

ButtonsTask::ButtonsTask()
{
    idleCounter = 0;
    longPressCounter = 0;

    buttonsMask = 0;
    buttonsOverride = 0;

    numHandlers = 0;

    PeriodicTask::init(ButtonPollCycle);
}

ButtonsTask * ButtonsTask::getInstance()
{
    static ButtonsTask instance;
    return &instance;
}

void ButtonsTask::start()
{
    startTimer(ButtonPollCycle);
}

void ButtonsTask::setButtonsOverride(uint32 override)
{
    buttonsOverride = override;
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
    uint32 input = ~u32AHI_DioReadInput() & buttonsMask;
    input |= buttonsOverride;

    bool someButtonPressed = false;                 // Used to reset idle counter
    bool allButtonsPressed = input == buttonsMask;  // Used to initiate join/leave

    // DBG_vPrintf(TRUE, "Input=%08x\n", input);
    for(uint8 h = 0; h < numHandlers; h++)
    {
        bool pressed = (input == handlers[h].pinMask);
        // DBG_vPrintf(TRUE, "PinMask=%08x pressed=%d\n", handlers[h].pinMask, pressed);
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

    // Process a very long press of all buttons to join/leave the network.
    // Join and leave use asymmetric thresholds so that an everyday long press (e.g. a
    // hold-to-dim gesture routed through the button) can never accidentally leave the network:
    //   - JOIN : all buttons held ~5s while NOT on a network (a fresh device is not running
    //            any hold gesture, so a short threshold is safe and convenient).
    //   - LEAVE: all buttons held ~30s while joined - deliberately long. Prefer removing the
    //            device from the coordinator (e.g. Zigbee2MQTT "Remove device") over this
    //            local escape hatch, which mainly exists for an orphaned device.
    if(allButtonsPressed)
    {
        ZigbeeDevice * zigbeeDevice = ZigbeeDevice::getInstance();
        bool doJoin  = !zigbeeDevice->isJoined() && longPressCounter > 5000/ButtonPollCycle;
        bool doLeave =  zigbeeDevice->isJoined() && longPressCounter > 30000/ButtonPollCycle;

        if(doJoin || doLeave)
        {
            for(uint8 h = 0; h < numHandlers; h++)
                handlers[h].handler->resetButtonStateMachine();

            longPressCounter = 0;

            if(doJoin)
                zigbeeDevice->joinNetwork();
            else
                zigbeeDevice->leaveNetwork();
        }
    }
}



#include "ButtonsTask.h"
#include "AppQueue.h"

#define BOARD_BTN_BIT               (1)
#define BOARD_BTN_PIN               (1UL << BOARD_BTN_BIT)

static const uint32 ButtonPollCycle = 10;

// Note: Object constructors are not executed by CRT if creating a global var of this object :(
// So has to be created explicitely in vAppMain() otherwise VTABLE will not be initialized properly

ButtonsTask::ButtonsTask()
{
    idleCounter = 0;
    currentState = IDLE;
    currentStateDuration = 0;

    switchType = SWITCH_TYPE_TOGGLE;
    switchMode = SWITCH_MODE_FRONT;
    maxPause = 30;
    longPressDuration = 100;

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
    return idleCounter > 5000 / ButtonPollCycle; // 500 cycles * 10 ms = 5 sec
}

inline void ButtonsTask::sendButtonEvent(ApplicationEventType evtType, uint8 button)
{
    ApplicationEvent evt = {evtType, button};
    appEventQueue.send(evt);
}

const char * ButtonsTask::getStateName(ButtonState state)
{
    switch(state)
    {
    case IDLE: return "IDLE";
    case PRESSED1: return "PRESSED1";
    case PAUSE1: return "PAUSE1";
    case PRESSED2: return "PRESSED2";
    case PAUSE2: return "PAUSE2";
    case PRESSED3: return "PRESSED3";
    case LONG_PRESS: return "LONG_PRESS";
    }

    // Should never happen
    return "";
}

void ButtonsTask::switchState(ButtonState state)
{
    currentState = state;
    currentStateDuration = 0;

    DBG_vPrintf(TRUE, "Switching button state to %s\n", getStateName(state));
}

void ButtonsTask::buttonStateMachineToggle(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                switchState(PRESSED1);
                sendButtonEvent(BUTTON_ACTION_SINGLE, 0);

                if(switchMode == SWITCH_MODE_FRONT)
                    sendButtonEvent(SWITCH_TRIGGER, 0);
            }
            break;

        case PRESSED1:
            if(!pressed)
                switchState(IDLE);

            break;

        default:
            switchState(IDLE);  // How did we get here?
            break;
    }
}

void ButtonsTask::buttonStateMachineMomentary(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                switchState(PRESSED1);
                sendButtonEvent(BUTTON_PRESSED, 0);

                if(switchMode == SWITCH_MODE_FRONT)
                    sendButtonEvent(SWITCH_ON, 0);
            }
            break;

        case PRESSED1:
            if(!pressed)
            {
                switchState(IDLE);
                sendButtonEvent(BUTTON_RELEASED, 0);

                if(switchMode == SWITCH_MODE_FRONT)
                    sendButtonEvent(SWITCH_OFF, 0);
            }

            break;

        default:
            switchState(IDLE); // How did we get here?
            break;
    }
}

void ButtonsTask::buttonStateMachineMultifunction(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                switchState(PRESSED1);

                if(switchMode == SWITCH_MODE_FRONT)
                    sendButtonEvent(SWITCH_TRIGGER, 0);
            }
            break;

        case PRESSED1:
            if(pressed && currentStateDuration > longPressDuration)
            {
                switchState(LONG_PRESS);
                sendButtonEvent(BUTTON_PRESSED, 0);

                if(switchMode == SWITCH_MODE_LONG)
                    sendButtonEvent(SWITCH_TRIGGER, 0);
            }

            if(!pressed)
            {
                switchState(PAUSE1);
            }

            break;

        case PAUSE1:
            if(!pressed && currentStateDuration > maxPause)
            {
                switchState(IDLE);
                sendButtonEvent(BUTTON_ACTION_SINGLE, 0);

                if(switchMode == SWITCH_MODE_SINGLE)
                    sendButtonEvent(SWITCH_TRIGGER, 0);
            }

            if(pressed)
                switchState(PRESSED2);

            break;

        case PRESSED2:
            if(!pressed)
            {
                switchState(PAUSE2);
            }

            break;

        case PAUSE2:
            if(!pressed && currentStateDuration > maxPause)
            {
                switchState(IDLE);
                sendButtonEvent(BUTTON_ACTION_DOUBLE, 0);

                if(switchMode == SWITCH_MODE_DOUBLE)
                    sendButtonEvent(SWITCH_TRIGGER, 0);
            }

            if(pressed)
            {
                switchState(PRESSED3);
            }

            break;

        case PRESSED3:
            if(!pressed)
            {
                switchState(IDLE);

                if(switchMode == SWITCH_MODE_TRIPPLE)
                    sendButtonEvent(SWITCH_TRIGGER, 0);

                sendButtonEvent(BUTTON_ACTION_TRIPPLE, 0);
            }

            break;

        case LONG_PRESS:
            if(!pressed)
            {
                switchState(IDLE);

                sendButtonEvent(BUTTON_RELEASED, 0);
            }

            break;

        default: break;
    }
}

void ButtonsTask::timerCallback()
{
    uint32 input = u32AHI_DioReadInput();
    bool pressed = (input & BOARD_BTN_PIN) == 0;

    // Reset the idle counter when user interacts with the button
    if(pressed)
        idleCounter = 0;
    else
        idleCounter++;


    // Let at least 20ms to stabilize button value, do not make any early decisions
    // When button state is stabilized - go through the corresponding state machine
    currentStateDuration++;
    if(currentStateDuration >= 2)
    {
        switch(switchType)
        {
        case SWITCH_TYPE_TOGGLE:
            buttonStateMachineToggle(pressed);
            break;
        case SWITCH_TYPE_MOMENTARY:
            buttonStateMachineMomentary(pressed);
            break;
        case SWITCH_TYPE_MULTIFUNCTION:
            buttonStateMachineMultifunction(pressed);
            break;
        default:
            break;
        }
    }

    // Process a very long press to join/leave the network
    if(pressed && currentStateDuration > 5000/ButtonPollCycle)
    {
        sendButtonEvent(BUTTON_VERY_LONG_PRESS, 0);
        switchState(IDLE);
    }

    startTimer(ButtonPollCycle);
}



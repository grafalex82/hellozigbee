#include "ButtonHandler.h"
#include "SwitchEndpoint.h"

extern "C"
{
    #include "AppHardwareApi.h"
}


ButtonHandler::ButtonHandler()
{
    endpoint = NULL;

    currentState = IDLE;
    currentStateDuration = 0;

    switchMode = SWITCH_MODE_TOGGLE;
    relayMode = RELAY_MODE_FRONT;
    maxPause = 250/ButtonPollCycle;
    longPressDuration = 1000/ButtonPollCycle;
}

void ButtonHandler::setEndpoint(SwitchEndpoint * ep)
{
    endpoint = ep;
}

inline void ButtonHandler::sendButtonEvent(ApplicationEventType evtType)
{
//TODO Handle single/double/tripple/long presses here
}

const char * ButtonHandler::getStateName(ButtonState state)
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

void ButtonHandler::setSwitchMode(SwitchMode mode)
{
    switchMode = mode;
    changeState(IDLE);
}

void ButtonHandler::setRelayMode(RelayMode mode)
{
    relayMode = mode;
    changeState(IDLE);
}

void ButtonHandler::setMaxPause(uint16 value)
{
    maxPause = value/ButtonPollCycle;
    changeState(IDLE);
}

void ButtonHandler::setMinLongPress(uint16 value)
{
    longPressDuration = value/ButtonPollCycle;
    changeState(IDLE);
}

void ButtonHandler::changeState(ButtonState state)
{
    currentState = state;
    currentStateDuration = 0;

    DBG_vPrintf(TRUE, "Switching button %d state to %s\n", endpoint->getEndpointId(), getStateName(state));
}

void ButtonHandler::buttonStateMachineToggle(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                changeState(PRESSED1);
                endpoint->reportAction(BUTTON_ACTION_SINGLE);

                if(relayMode != RELAY_MODE_UNLINKED)
                    endpoint->toggle();
            }
            break;

        case PRESSED1:
            if(!pressed)
                changeState(IDLE);

            break;

        default:
            changeState(IDLE);  // How did we get here?
            break;
    }
}

void ButtonHandler::buttonStateMachineMomentary(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                changeState(PRESSED1);
                endpoint->reportAction(BUTTON_PRESSED);

                if(relayMode != RELAY_MODE_UNLINKED)
                    endpoint->switchOn();

                endpoint->reportLongPress(true);
            }
            break;

        case PRESSED1:
            if(!pressed)
            {
                changeState(IDLE);
                endpoint->reportAction(BUTTON_RELEASED);

                if(relayMode != RELAY_MODE_UNLINKED)
                    endpoint->switchOff();

                endpoint->reportLongPress(false);
            }

            break;

        default:
            changeState(IDLE); // How did we get here?
            break;
    }
}

void ButtonHandler::buttonStateMachineMultistate(bool pressed)
{
    // The state machine
    switch(currentState)
    {
        case IDLE:
            if(pressed)
            {
                changeState(PRESSED1);

                if(relayMode == RELAY_MODE_FRONT)
                    endpoint->toggle();
            }
            break;

        case PRESSED1:
            if(pressed && currentStateDuration > longPressDuration)
            {
                changeState(LONG_PRESS);
                endpoint->reportAction(BUTTON_PRESSED);

                if(relayMode == RELAY_MODE_LONG)
                    endpoint->toggle();

                endpoint->reportLongPress(true);
            }

            if(!pressed)
            {
                changeState(PAUSE1);
            }

            break;

        case PAUSE1:
            if(!pressed && currentStateDuration > maxPause)
            {
                changeState(IDLE);
                endpoint->reportAction(BUTTON_ACTION_SINGLE);

                if(relayMode == RELAY_MODE_SINGLE)
                    endpoint->toggle();
            }

            if(pressed)
                changeState(PRESSED2);

            break;

        case PRESSED2:
            if(!pressed)
            {
                changeState(PAUSE2);
            }

            break;

        case PAUSE2:
            if(!pressed && currentStateDuration > maxPause)
            {
                changeState(IDLE);
                endpoint->reportAction(BUTTON_ACTION_DOUBLE);

                if(relayMode == RELAY_MODE_DOUBLE)
                    endpoint->toggle();
            }

            if(pressed)
            {
                changeState(PRESSED3);
            }

            break;

        case PRESSED3:
            if(!pressed)
            {
                changeState(IDLE);

                if(relayMode == RELAY_MODE_TRIPPLE)
                    endpoint->toggle();

                endpoint->reportAction(BUTTON_ACTION_TRIPPLE);
            }

            break;

        case LONG_PRESS:
            if(!pressed)
            {
                changeState(IDLE);

                endpoint->reportAction(BUTTON_RELEASED);
                endpoint->reportLongPress(false);
            }

            break;

        default: break;
    }
}

void ButtonHandler::handleButtonState(bool pressed)
{
    // Let at least 20ms to stabilize button value, do not make any early decisions
    // When button state is stabilized - go through the corresponding state machine
    currentStateDuration++;
    if(currentStateDuration < 2)
        return;

    switch(switchMode)
    {
    case SWITCH_MODE_TOGGLE:
        buttonStateMachineToggle(pressed);
        break;
    case SWITCH_MODE_MOMENTARY:
        buttonStateMachineMomentary(pressed);
        break;
    case SWITCH_MODE_MULTIFUNCTION:
        buttonStateMachineMultistate(pressed);
        break;
    default:
        break;
    }
}

void ButtonHandler::resetButtonStateMachine()
{
    changeState(IDLE);
}

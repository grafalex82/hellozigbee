#include "ButtonHandler.h"
#include "SwitchEndpoint.h"

extern "C"
{
    #include "dbg.h"
    #include "AppHardwareApi.h"
}


ButtonHandler::ButtonHandler()
{
    endpoint = NULL;

    prevState = false;
    debounceTimer = 0;

    currentState = INVALID;
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
    case INVALID: return "INVALID";
    }

    // Should never happen
    return "";
}

void ButtonHandler::setConfiguration(SwitchMode sMode, RelayMode rMode, uint16 maxPause, uint16 minLongPress)
{
    // This function does the same as 4 functions below all together, but as a single transaction.
    // This is needed to avoid cluttering log with multiple changeState messages
    switchMode = sMode;
    relayMode = rMode;
    maxPause = maxPause/ButtonPollCycle;
    longPressDuration = minLongPress/ButtonPollCycle;

    changeState(INVALID, true);
}

void ButtonHandler::setSwitchMode(SwitchMode mode)
{
    switchMode = mode;
    changeState(INVALID);
}

void ButtonHandler::setRelayMode(RelayMode mode)
{
    relayMode = mode;
    changeState(INVALID);
}

void ButtonHandler::setMaxPause(uint16 value)
{
    maxPause = value/ButtonPollCycle;
    changeState(INVALID);
}

void ButtonHandler::setMinLongPress(uint16 value)
{
    longPressDuration = value/ButtonPollCycle;
    changeState(INVALID);
}

void ButtonHandler::changeState(ButtonState state, bool suppressLogging)
{
    currentState = state;
    currentStateDuration = 0;

    // TODO: Avoid dumping multiple changeState() calls during initial initialization
    if(!suppressLogging)
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
            if(pressed && (currentStateDuration > longPressDuration))
            {
                changeState(LONG_PRESS);
                endpoint->reportAction(BUTTON_PRESSED);

                if(relayMode == RELAY_MODE_LONG)
                    endpoint->switchOn();

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

                endpoint->reportAction(BUTTON_ACTION_TRIPPLE);

                if(relayMode == RELAY_MODE_TRIPPLE)
                    endpoint->toggle();

            }

            break;

        case LONG_PRESS:
            if(!pressed)
            {
                changeState(IDLE);

                endpoint->reportAction(BUTTON_RELEASED);

                if(relayMode == RELAY_MODE_LONG)
                    endpoint->switchOff();
                    
                endpoint->reportLongPress(false);
            }

            break;

        default: break;
    }
}

void ButtonHandler::handleButtonState(bool pressed)
{
    // Let at least 60ms to stabilize button value, do not make any early decisions
    // When button state is stabilized - go through the corresponding state machine
    if(pressed != prevState)
    {
        prevState = pressed;
        debounceTimer = 0;
    }

    debounceTimer++;
    if(debounceTimer <= 3)
        return;

    // Increment current state duration - some states use this variable to calculate timings before switching to a new state
    currentStateDuration++;

    // On a mode change the state is set to INVALID. This is needed to avoid immediate handling of a pressed button (if any).
    // This check performs exit from INVALID state to IDLE upon button release
    if(currentState == INVALID)
    {
        if(!pressed)
            changeState(IDLE);
            
        return;
    }

    // Handle the button according to the current mode
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
    changeState(INVALID);
}

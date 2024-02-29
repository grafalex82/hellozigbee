#ifndef BUTTONHANDLER_H
#define BUTTONHANDLER_H

#include "ButtonModes.h"
#include "IButtonHandler.h"

#include <jendefs.h>

class SwitchEndpoint;

typedef enum
{
    BUTTON_RELEASED 		= 0,
    BUTTON_ACTION_SINGLE 	= 1,
    BUTTON_ACTION_DOUBLE	= 2,
    BUTTON_ACTION_TRIPPLE	= 3, 
    BUTTON_PRESSED		= 255
} ButtonActionType;


class ButtonHandler: public IButtonHandler
{
    SwitchEndpoint * endpoint;

    bool prevState;
    uint32 debounceTimer;
    uint32 currentStateDuration;

    SwitchMode switchMode;
    RelayMode relayMode;
    uint16 maxPause;
    uint16 longPressDuration;

    enum ButtonState
    {
        IDLE,
        PRESSED1,
        PAUSE1,
        PRESSED2,
        PAUSE2,
        PRESSED3,
        LONG_PRESS,
        INVALID
    };

    ButtonState currentState;

public:
    ButtonHandler();

    void setEndpoint(SwitchEndpoint * ep);

    void setConfiguration(SwitchMode switchMode, RelayMode relayMode, uint16 maxPause, uint16 minLongPress);
    void setSwitchMode(SwitchMode mode);
    void setRelayMode(RelayMode mode);
    void setMaxPause(uint16 value);
    void setMinLongPress(uint16 value);

    void resetButtonStateMachine();

protected:
    virtual void handleButtonState(bool pressed);

    virtual void changeState(ButtonState state, bool suppressLogging = false);
    virtual void buttonStateMachineToggle(bool pressed);
    virtual void buttonStateMachineMomentary(bool pressed);
    virtual void buttonStateMachineMultistate(bool pressed);

    const char * getStateName(ButtonState state);
};

#endif // BUTTONHANDLER_H

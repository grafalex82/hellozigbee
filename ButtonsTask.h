#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

#include "ButtonModes.h"
#include "PeriodicTask.h"
#include "Queue.h"
#include "AppQueue.h"

class ButtonsTask : public PeriodicTask
{
    uint32 idleCounter;
    uint32 currentStateDuration;

    LocalSwitchMode switchMode;
    ButtonMode buttonMode;
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
        LONG_PRESS
    };

    ButtonState currentState;

public:
    ButtonsTask();

    static ButtonsTask * getInstance();

    bool handleDioInterrupt(uint32 dioStatus);
    bool canSleep() const;

protected:
    virtual void timerCallback();
    virtual void switchState(ButtonState state);
    virtual void buttonStateMachine(bool pressed);
    void sendButtonEvent(ApplicationEventType evtType, uint8 button);

    const char * getStateName(ButtonState state);
};

#endif // BUTTONSTASK_H

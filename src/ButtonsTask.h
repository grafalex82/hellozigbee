#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

extern "C"
{
    #include "zcl.h"
    #include "zcl_options.h"
}

#include "PeriodicTask.h"
#include "Queue.h"

class IButtonHandler;

struct HandlerRecord
{
    uint32 pinMask;
    IButtonHandler * handler;
};

class ButtonsTask : public PeriodicTask
{
    uint32 idleCounter;
    uint32 longPressCounter;

    HandlerRecord handlers[ZCL_NUMBER_OF_ENDPOINTS+1];
    uint8 numHandlers;
    uint32 buttonsMask;
    uint32 buttonsOverride;

    ButtonsTask();

public:
    static ButtonsTask * getInstance();
    void start();

    void setButtonsOverride(uint32 override);
    
    bool handleDioInterrupt(uint32 dioStatus);
    bool canSleep() const;

    void registerHandler(uint32 pinMask, IButtonHandler * handler);

protected:
    virtual void timerCallback();
};

#endif // BUTTONSTASK_H

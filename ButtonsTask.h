#ifndef BUTTONSTASK_H
#define BUTTONSTASK_H

#include "PeriodicTask.h"
#include "Queue.h"

extern "C"
{
    #include "zcl.h"
    #include "zcl_options.h"
}

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

public:
    ButtonsTask();

    static ButtonsTask * getInstance();

    bool handleDioInterrupt(uint32 dioStatus);
    bool canSleep() const;

    void registerHandler(uint32 pinMask, IButtonHandler * handler);

protected:
    virtual void timerCallback();
};

#endif // BUTTONSTASK_H

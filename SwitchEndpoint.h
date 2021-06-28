#ifndef SWITCH_ENDPOINT_H
#define SWITCH_ENDPOINT_H

extern "C"
{
    #include "zcl.h"
    #include "on_off_light_switch.h"
}

#include "Endpoint.h"
#include "BlinkTask.h"

class SwitchEndpoint: public Endpoint
{    
protected:
    tsZLO_OnOffLightSwitchDevice sSwitch;
    BlinkTask blinkTask;

public:
    SwitchEndpoint();
    virtual void init();

    bool getState() const;
    void switchOn();
    void switchOff();
    void toggle();

protected:
    void doStateChange(bool state);
    void reportStateChange();

protected:
    virtual void handleClusterUpdate(tsZCL_CallBackEvent *psEvent);
};

#endif // SWITCH_ENDPOINT_H

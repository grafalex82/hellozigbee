#ifndef SWITCH_ENDPOINT_H
#define SWITCH_ENDPOINT_H

extern "C"
{
    #include "jendefs.h"
    #include "zps_gen.h"
    #include "zcl.h"
    #include "OnOff.h"
    #include "OOSC.h"
    #include "MultistateInputBasic.h"
    #include "LevelControl.h"
}

#include "Endpoint.h"
#include "BlinkTask.h"
#include "ButtonHandler.h"

// List of cluster instances (descriptor objects) that are included into the endpoint
struct OnOffClusterInstances
{
    tsZCL_ClusterInstance sOnOffClient;
    tsZCL_ClusterInstance sOnOffServer;
    tsZCL_ClusterInstance sOnOffConfigServer;
    tsZCL_ClusterInstance sMultistateInputServer;
    tsZCL_ClusterInstance sLevelControlClient;
} __attribute__ ((aligned(4)));


class SwitchEndpoint: public Endpoint
{    
protected:
    tsZCL_EndPointDefinition sEndPoint;
    OnOffClusterInstances sClusterInstance;
    tsCLD_OnOff sOnOffClientCluster;
    tsCLD_OnOff sOnOffServerCluster;
    tsCLD_OOSC sOnOffConfigServerCluster;
    tsCLD_OnOffCustomDataStructure sOnOffServerCustomDataStructure;
    tsCLD_MultistateInputBasic sMultistateInputServerCluster;
    tsCLD_LevelControl sLevelControlClientCluster;
    tsCLD_LevelControlCustomDataStructure sLevelControlClientCustomDataStructure;

    BlinkTask blinkTask;
    ButtonHandler buttonHandler;

public:
    SwitchEndpoint();
    void setPins(uint8 ledPin, uint32 pinMask);
    virtual void init();

    bool getState() const;
    void switchOn();
    void switchOff();
    void toggle();
    bool runsInServerMode() const;

    void reportAction(ButtonActionType action);
    void reportLongPress(bool pressed);

protected:
    void doStateChange(bool state);
    void reportState();
    void sendCommandToBoundDevices();
    void sendLevelControlMoveCommand(bool up);
    void sendLevelControlStopCommand();
    void reportStateChange();

protected:
    virtual void registerServerCluster();
    virtual void registerClientCluster();
    virtual void registerOnOffConfigServerCluster();
    virtual void registerMultistateInputServerCluster();
    virtual void registerLevelControlClientCluster();
    virtual void registerEndpoint();
    virtual void restoreConfiguration();
    virtual void saveConfiguration();
    virtual void handleClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual void handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent);
};

#endif // SWITCH_ENDPOINT_H

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
    #include "Identify.h"
    #include "Groups.h"
}

#include "Endpoint.h"
#include "ButtonHandler.h"

// A reporting configuration record to save in PDM
struct ReportConfiguration
{
    uint8 clusterID;
    tsZCL_AttributeReportingConfigurationRecord record;
};

// List of cluster instances (descriptor objects) that are included into the endpoint
// The clusters defined in this list work for both client and server modes. 
//
// Note: Do not mix up endpoint client/server mode with individual cluster client/server instance
//       The switch client mode means it can work only like a controller device (e.g. smart button), but does not
//       have relay control and internal state. At the same time it has number of server clusters (Multistate Input,
//       OOSC) for configuration and reporting actions.
struct OnOffClientClusterInstances
{
    tsZCL_ClusterInstance sOnOffClient;
    tsZCL_ClusterInstance sOnOffConfigServer;
    tsZCL_ClusterInstance sMultistateInputServer;
    tsZCL_ClusterInstance sLevelControlClient;
    tsZCL_ClusterInstance sIdentifyServer;
} __attribute__ ((aligned(4)));

// List of additional clusters for server mode
// Note: In server mode the switch has its internal state, can report state attributes, and can be assigned to a group
struct OnOffServerClusterInstances
{
    tsZCL_ClusterInstance sOnOffServer;
    tsZCL_ClusterInstance sGroupsServer;
} __attribute__ ((aligned(4)));

// This structure is used just to ensure client and server clusters are layed out consecutively in memory
struct OnOffClusterInstances
{
    OnOffClientClusterInstances client;
    OnOffServerClusterInstances server;
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
    tsCLD_LevelControlClient sLevelControlClientCluster;
    tsCLD_LevelControlCustomDataStructure sLevelControlClientCustomDataStructure;
    tsCLD_Identify sIdentifyServerCluster;
    tsCLD_IdentifyCustomDataStructure sIdentifyClusterData;
    tsCLD_Groups sGroupsServerCluster;
    tsCLD_GroupsCustomDataStructure sGroupsServerCustomDataStructure;

    ButtonHandler buttonHandler;
    bool clientOnly;
    SwitchEndpoint * interlockBuddy;
    ReportConfiguration reportConfigurations[ZCL_NUMBER_OF_REPORTS];

public:
    SwitchEndpoint();
    void setConfiguration(uint32 pinMask, bool disableServer = false);
    void setInterlockBuddy(SwitchEndpoint * buddy);
    virtual void init();

    bool getState() const;
    void switchOn();
    void switchOff();
    void toggle();
    bool runsInServerMode() const;

    void reportAction(ButtonActionType action);
    void reportLongPress(bool pressed);

    void setInterlockMode(teCLD_OOSC_InterlockMode mode);
    void setInterlockState(bool buddyState);

protected:
    void doStateChange(bool state, bool sourcedFromInterlockBuddy = false);
    void reportState();
    void sendCommandToBoundDevices(teCLD_OnOff_Command cmd);
    void sendLevelControlMoveCommand(bool up);
    void sendLevelControlStopCommand();

protected:
    virtual void initEndpointStructure();
    virtual void registerServerCluster();
    virtual void registerClientCluster();
    virtual void registerGroupsCluster();
    virtual void registerOnOffConfigServerCluster();
    virtual void registerMultistateInputServerCluster();
    virtual void registerLevelControlClientCluster();
    virtual void registerIdentifyCluster();
    virtual void registerEndpoint();

    virtual void restoreButtonsConfiguration();
    virtual void saveButtonsConfiguration();

    virtual void initReportingConfigurations();
    virtual void saveReportingConfigurations();
    virtual void restoreReportingConfigurations();
    virtual uint8 findReportingConfiguration(uint16 clusterID, uint16 attributeID);
    virtual void addReportConfiguration(uint16 clusterID, tsZCL_AttributeReportingConfigurationRecord * record);

    virtual void handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent);
    virtual void handleOnOffClusterCommand(tsZCL_CallBackEvent *psEvent);
    virtual void handleIdentifyClusterCommand(tsZCL_CallBackEvent *psEvent);
    virtual void handleGroupsClusterCommand(tsZCL_CallBackEvent *psEvent);

    virtual void handleClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual void handleOnOffClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual void handleIdentifyClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual void handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent);

    virtual teZCL_CommandStatus handleCheckAttributeRange(tsZCL_CallBackEvent *psEvent);
    virtual void handleReportingConfigureRequest(tsZCL_CallBackEvent *psEvent);

    virtual void handleDeviceJoin();
    virtual void handleDeviceLeave();
};

#endif // SWITCH_ENDPOINT_H

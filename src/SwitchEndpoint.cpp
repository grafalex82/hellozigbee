#include "SwitchEndpoint.h"

#include "DumpFunctions.h"
#include "EndpointManager.h"
#include "ZigbeeDevice.h"
#include "ButtonsTask.h"
#include "PdmIds.h"
#include "LEDTask.h"
#include "RelayTask.h"

extern "C"
{
    #include "dbg.h"
    #include "string.h"
    #include "zcl_customcommand.h"
    #include "PDM.h"
}

static const uint8 PARAM_ID_BUTTON_CONFIG = 0;
static const uint8 PARAM_ID_REPORTING_CONFIG = 1;


SwitchEndpoint::SwitchEndpoint()
{
}

void SwitchEndpoint::setConfiguration(uint32 pinMask, bool disableServer)
{
    interlockBuddy = NULL;
    clientOnly = disableServer;
    ButtonsTask::getInstance()->registerHandler(pinMask, &buttonHandler);
}

void SwitchEndpoint::setInterlockBuddy(SwitchEndpoint * buddy)
{
    interlockBuddy = buddy;
}

void SwitchEndpoint::registerServerCluster()
{
    // Initialize On/Off server cluser
    teZCL_Status status = eCLD_OnOffCreateOnOff(&sClusterInstance.server.sOnOffServer,
                                                TRUE,                               // Server
                                                &sCLD_OnOff,
                                                &sOnOffServerCluster,
                                                &au8OnOffAttributeControlBits[0],
                                                &sOnOffServerCustomDataStructure);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerClientCluster()
{
    // Initialize On/Off client cluser
    teZCL_Status status = eCLD_OnOffCreateOnOff(&sClusterInstance.client.sOnOffClient,
                                                FALSE,                              // Client
                                                &sCLD_OnOff,
                                                &sOnOffClientCluster,
                                                &au8OnOffAttributeControlBits[0],
                                                NULL);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff client cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerOnOffConfigServerCluster()
{
    // Initialize On/Off config server cluser
    teZCL_Status status = eCLD_OOSCCreateOnOffSwitchConfig(&sClusterInstance.client.sOnOffConfigServer,
                                                           TRUE,                              // Server
                                                           &sCLD_OOSC,
                                                           &sOnOffConfigServerCluster,
                                                           &au8OOSCAttributeControlBits[0]);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff config server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerMultistateInputServerCluster()
{
    // Initialize Multistate Input server cluser
    teZCL_Status status = eCLD_MultistateInputBasicCreateMultistateInputBasic(
                &sClusterInstance.client.sMultistateInputServer,
                TRUE,                              // Server
                &sCLD_MultistateInputBasic,
                &sMultistateInputServerCluster,
                &au8MultistateInputBasicAttributeControlBits[0]);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create Multistate Input server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerLevelControlClientCluster()
{
    // Initialize Level Control client cluser
    teZCL_Status status = eCLD_LevelControlCreateLevelControl(&sClusterInstance.client.sLevelControlClient,
                                                              FALSE,                              // Client
                                                              &sCLD_LevelControlClient,
                                                              &sLevelControlClientCluster,
                                                              &au8LevelControlClientAttributeControlBits[0],
                                                              &sLevelControlClientCustomDataStructure);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create Level Control client cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerIdentifyCluster()
{
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_IdentifyCreateIdentify(&sClusterInstance.client.sIdentifyServer,
                                                TRUE,
                                                &sCLD_Identify,
                                                &sIdentifyServerCluster,
                                                &au8IdentifyAttributeControlBits[0],
                                                &sIdentifyClusterData);
    
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::registerIdentifyCluster(): Failed to create Identify Cluster instance. Status=%d\n", status);
}

void SwitchEndpoint::registerGroupsCluster()
{
    // Create an instance of a groups cluster as a server
    teZCL_Status status = eCLD_GroupsCreateGroups(&sClusterInstance.server.sGroupsServer,
                                                  TRUE,
                                                  &sCLD_Groups,
                                                  &sGroupsServerCluster,
                                                  &au8GroupsAttributeControlBits[0],
                                                  &sGroupsServerCustomDataStructure,
                                                  &sEndPoint);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create Groups Cluster instance. status=%d\n", status);
}

void SwitchEndpoint::initEndpointStructure()
{
    // Calculate number of clusters, depending on client/server mode
    uint16 numClusters = sizeof(OnOffClientClusterInstances) / sizeof(tsZCL_ClusterInstance);
    if(!clientOnly)
        numClusters = sizeof(OnOffClusterInstances) / sizeof(tsZCL_ClusterInstance);

    // Initialize endpoint structure
    sEndPoint.u8EndPointNumber = getEndpointId();
    sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    sEndPoint.u16NumberOfClusters = numClusters;
    sEndPoint.psClusterInstance = (tsZCL_ClusterInstance*)&sClusterInstance;
    sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    sEndPoint.pCallBackFunctions = &EndpointManager::handleZclEvent;
}

void SwitchEndpoint::registerEndpoint()
{
    // Register the endpoint with all the clusters in it
    teZCL_Status status = eZCL_Register(&sEndPoint);
    DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Register Switch Endpoint. status=%d\n", status);
}

void SwitchEndpoint::restoreButtonsConfiguration()
{
    // Read values from PDM
    uint16 readBytes;
    PDM_eReadDataFromRecord(getPdmIdForEndpoint(getEndpointId(), PARAM_ID_BUTTON_CONFIG),
                            &sOnOffConfigServerCluster,
                            sizeof(sOnOffConfigServerCluster),
                            &readBytes);

    // Configure buttons state machine with read values
    buttonHandler.setConfiguration((SwitchMode)sOnOffConfigServerCluster.eSwitchMode, 
                                   (RelayMode)sOnOffConfigServerCluster.eRelayMode,
                                   sOnOffConfigServerCluster.iMaxPause,
                                   sOnOffConfigServerCluster.iMinLongPress);

    // Make sure that client only endpoints have client mode set
    if(clientOnly)
        sOnOffConfigServerCluster.eOperationMode = E_CLD_OOSC_OPERATION_MODE_CLIENT;

    // Dump the restored configuration
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Restored buttons configuration:\n", getEndpointId());
    DBG_vPrintf(TRUE, "    Operation mode = %d\n", sOnOffConfigServerCluster.eOperationMode);
    DBG_vPrintf(TRUE, "    Switch mode = %d\n", sOnOffConfigServerCluster.eSwitchMode);
    DBG_vPrintf(TRUE, "    Relay mode = %d\n", sOnOffConfigServerCluster.eRelayMode);
    DBG_vPrintf(TRUE, "    Switch actions = %d\n", sOnOffConfigServerCluster.eSwitchActions);
    DBG_vPrintf(TRUE, "    Long press mode = %d\n", sOnOffConfigServerCluster.eLongPressMode);
}

void SwitchEndpoint::saveButtonsConfiguration()
{
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Save buttons configuration\n", getEndpointId());
    PDM_eSaveRecordData(getPdmIdForEndpoint(getEndpointId(), PARAM_ID_BUTTON_CONFIG),
                        &sOnOffConfigServerCluster,
                        sizeof(sOnOffConfigServerCluster));
}

void SwitchEndpoint::init()
{
    // Register all clusters and endpoint itself
    initEndpointStructure();
    registerClientCluster();
    registerOnOffConfigServerCluster();
    registerMultistateInputServerCluster();
    registerLevelControlClientCluster();
    registerIdentifyCluster();
    if(!clientOnly)
    {
        registerServerCluster();
        registerGroupsCluster();
    }
    registerEndpoint();

    // Let button handler know about this Endpoint instanct so that it can properly report new states
    buttonHandler.setEndpoint(this);

    // Restore previous configuration from PDM
    restoreButtonsConfiguration();
    restoreReportingConfigurations();
    // TODO: restore previous brightness from PDM
}

bool SwitchEndpoint::getState() const
{
    if(runsInServerMode())
        return sOnOffServerCluster.bOnOff;

    return false;
}

void SwitchEndpoint::switchOn()
{
    // Handle toggle mode separately
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_TOGGLE)
        return toggle();

    // Invert the value in inverse mode
    bool newState = true;
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_S2OFF_S1ON)
        newState = false;
    
    if(runsInServerMode())  
        // In Server mode we will switch internal relay
        doStateChange(newState);
    else 
        // In Client mode we will send On/Off command
        sendCommandToBoundDevices(newState ? E_CLD_ONOFF_CMD_ON : E_CLD_ONOFF_CMD_OFF);
}

void SwitchEndpoint::switchOff()
{
    // Handle toggle mode separately
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_TOGGLE)
        return toggle();

    // Invert the value in inverse mode
    bool newState = false;
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_S2OFF_S1ON)
        newState = true;

    if(runsInServerMode())  
        // In Server mode we will switch internal relay
        doStateChange(newState);
    else
        // In Client mode we will send On/Off command
        sendCommandToBoundDevices(newState ? E_CLD_ONOFF_CMD_ON : E_CLD_ONOFF_CMD_OFF);
}

void SwitchEndpoint::toggle()
{
    if(runsInServerMode())  
        // In Server mode we will toggle internal relay state
        doStateChange(!getState());
    else 
        // In Client mode we will send Toggle command
        sendCommandToBoundDevices(E_CLD_ONOFF_CMD_TOGGLE);
}

void SwitchEndpoint::doStateChange(bool state, bool sourcedFromInterlockBuddy)
{
    // Disabled in Client mode
    if(!runsInServerMode())
        return;

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: do state change %d\n", getEndpointId(), state);
    sOnOffServerCluster.bOnOff = state ? TRUE : FALSE;

    LEDTask::getInstance()->setFixedLevel(getEndpointId(), state ? 255 : 0);
    RelayTask::getInstance()->setState(getEndpointId(), state);
    reportState();

    // Let the buddy know about our state change
    if(!sourcedFromInterlockBuddy && interlockBuddy)
        interlockBuddy->setInterlockState(state);
}

void SwitchEndpoint::reportState()
{
    // Disabled in Client mode
    if(!runsInServerMode())
        return;

    // Can send reports only when connected
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore reporting the change.\n");
        return;
    }

    // Destination address - 0x0000 (coordinator)
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT;

    // Send the report
    DBG_vPrintf(TRUE, "Reporting state change for EP=%d: State=%d... ", getEndpointId(), sOnOffServerCluster.bOnOff);
    PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    teZCL_Status status = eZCL_ReportAttribute(&addr,
                                               GENERAL_CLUSTER_ID_ONOFF,
                                               E_CLD_ONOFF_ATTR_ID_ONOFF,
                                               getEndpointId(),
                                               1,
                                               myPDUM_thAPduInstance);
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRUE, "status: %02x\n", status);
}

void SwitchEndpoint::sendCommandToBoundDevices(teCLD_OnOff_Command cmd)
{
    // Can send commands only when connected
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore sending commands\n");
        return;
    }

    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    // Send the toggle command
    uint8 sequenceNo;
    teZCL_Status status = eCLD_OnOffCommandSend(getEndpointId(),
                                   1,
                                   &addr,
                                   &sequenceNo,
                                   cmd);
    DBG_vPrintf(TRUE, "Sending On/Off command status: %02x\n", status);
}

void SwitchEndpoint::reportLongPress(bool pressed)
{
    // Can send commands only when connected
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore sending LevelCtrl command.\n");
        return;
    }

    switch(sOnOffConfigServerCluster.eLongPressMode)
    {
        case E_CLD_OOSC_LONG_PRESS_MODE_LEVEL_CTRL_UP:
            if(pressed)
                sendLevelControlMoveCommand(true);
            else
                sendLevelControlStopCommand();
            break;

        case E_CLD_OOSC_LONG_PRESS_MODE_LEVEL_CTRL_DOWN:
            if(pressed)
                sendLevelControlMoveCommand(false);
            else
                sendLevelControlStopCommand();
            break;

        // TODO: Add more valuable options/commands here

        default:
            break;
    }
}

void SwitchEndpoint::sendLevelControlMoveCommand(bool up)
{
    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    // Send the move command
    uint8 sequenceNo;
    tsCLD_LevelControl_MoveCommandPayload payload = {
        up ? (uint8)0x00 : (uint8)0x01, // u8MoveMode
        80,                             // u8Rate
        0,                              // u8OptionsMask
        0                               // u8OptionsOverride
    };
    teZCL_Status status = eCLD_LevelControlCommandMoveCommandSend(getEndpointId(),
                                                                  1,
                                                                  &addr,
                                                                  &sequenceNo,
                                                                  up ? TRUE : FALSE,    // Up will turn on the light, down will not turn off
                                                                  &payload);
    DBG_vPrintf(TRUE, "Sending Level Control Move command status: %02x\n", status);
}

void SwitchEndpoint::sendLevelControlStopCommand()
{
    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND_NON_BLOCKING;

    // Send the move command
    uint8 sequenceNo;
    tsCLD_LevelControl_StopCommandPayload payload = {
        0,          // u8OptionsMask
        0           // u8OptionsOverride
    };
    teZCL_Status status = eCLD_LevelControlCommandStopCommandSend(getEndpointId(),
                                                                  1,
                                                                  &addr,
                                                                  &sequenceNo,
                                                                  FALSE,
                                                                  &payload);
    DBG_vPrintf(TRUE, "Sending Level Control Stop command status: %02x\n", status);
}

void SwitchEndpoint::reportAction(ButtonActionType action)
{
    // Store new value in the cluster
    sMultistateInputServerCluster.u16PresentValue = (zuint16)action;

    // Prevent bothering Zigbee API if not connected
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore reporting the change.\n");
        return;
    }

    // Destination address - 0x0000 (coordinator)
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT;

    // Send the report
    DBG_vPrintf(TRUE, "Reporting multistate action EP=%d value=%d... ", getEndpointId(), sMultistateInputServerCluster.u16PresentValue);
    PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
    teZCL_Status status = eZCL_ReportAttribute(&addr,
                                               GENERAL_CLUSTER_ID_MULTISTATE_INPUT_BASIC,
                                               E_CLD_MULTISTATE_INPUT_BASIC_ATTR_ID_PRESENT_VALUE,
                                               getEndpointId(),
                                               1,
                                               myPDUM_thAPduInstance);
    PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
    DBG_vPrintf(TRUE, "status: %02x\n", status);
}

void SwitchEndpoint::handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_ONOFF:
            handleOnOffClusterCommand(psEvent);
            break;

        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterCommand(psEvent);
            break;

        case GENERAL_CLUSTER_ID_GROUPS:
            handleGroupsClusterCommand(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "SwitchEndpoint: EP=%d: Warning: Unexpected custom cluster event ClusterID=%04x\n", 
                        getEndpointId(), clusterId);
            break;
    }
}

void SwitchEndpoint::handleOnOffClusterCommand(tsZCL_CallBackEvent *psEvent)
{
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 commandId = msg->u8CommandId;

    if(clientOnly){
        DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Warning: On/Off Cluster command received on CLIENT ONLY endpoint Cmd=%02x\n",
                psEvent->u8EndPoint,
                commandId);
        return;
    }

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: On/Off Cluster command received Cmd=%02x. Ignored.\n",
                psEvent->u8EndPoint,
                commandId);
}

void SwitchEndpoint::handleIdentifyClusterCommand(tsZCL_CallBackEvent *psEvent)
{
    tsCLD_IdentifyCallBackMessage * msg = (tsCLD_IdentifyCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 commandId = msg->u8CommandId;
    uint8 ep = psEvent->u8EndPoint;

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Identify cluster command Cmd=%d\n", ep, commandId);

    switch(commandId)
    {
        case E_CLD_IDENTIFY_CMD_IDENTIFY:
            LEDTask::getInstance()->triggerEffect(ep, E_CLD_IDENTIFY_EFFECT_BREATHE);
            break;

        case E_CLD_IDENTIFY_CMD_TRIGGER_EFFECT:
            LEDTask::getInstance()->triggerEffect(ep, msg->uMessage.psTriggerEffectRequestPayload->eEffectId);
            break;

        default:
            break;
    }
}

void SwitchEndpoint::handleGroupsClusterCommand(tsZCL_CallBackEvent *psEvent)
{
    tsCLD_GroupsCallBackMessage * msg = (tsCLD_GroupsCallBackMessage*)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 commandId = msg->u8CommandId;
    uint8 ep = psEvent->u8EndPoint;

    if(clientOnly) {
        DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Warning: Groups cluster command Cmd=%d received on CLIENT ONLY endpoint\n", ep, commandId);
        return;
    }

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Groups cluster command Cmd=%d\n", ep, commandId);
}

void SwitchEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Cluster update message ClusterID=%04x\n",
                psEvent->u8EndPoint,
                clusterId);

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_ONOFF:
            handleOnOffClusterUpdate(psEvent);
            break;

        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterUpdate(psEvent);
            break;

        default:
            break;
    }
}

void SwitchEndpoint::handleOnOffClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: On/Off update message. New state: %d\n",
                psEvent->u8EndPoint,
                sOnOffServerCluster.bOnOff);

    doStateChange(getState());
}

void SwitchEndpoint::handleIdentifyClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    zuint16 identifyTime = sIdentifyServerCluster.u16IdentifyTime;
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Identify cluster update event. Identify Time = %d\n",
                psEvent->u8EndPoint, 
                identifyTime);

    if(identifyTime == 0)
        LEDTask::getInstance()->stopEffect();
}

void SwitchEndpoint::handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;

    // Update buttons state machine with received value
    if(clusterId == GENERAL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION)
    {
        switch(attrId)
        {
            case E_CLD_OOSC_ATTR_ID_SWITCH_MODE:
                buttonHandler.setSwitchMode((SwitchMode)sOnOffConfigServerCluster.eSwitchMode);
                break;

            case E_CLD_OOSC_ATTR_ID_SWITCH_RELAY_MODE:
                buttonHandler.setRelayMode((RelayMode)sOnOffConfigServerCluster.eRelayMode);
                break;

            case E_CLD_OOSC_ATTR_ID_SWITCH_MAX_PAUSE:
                buttonHandler.setMaxPause(sOnOffConfigServerCluster.iMaxPause);
                break;

            case E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_DUR:
                buttonHandler.setMinLongPress(sOnOffConfigServerCluster.iMinLongPress);
                break;

            case E_CLD_OOSC_ATTR_ID_SWITCH_INTERLOCK_MODE:
                if (interlockBuddy)
                {
                    interlockBuddy->setInterlockMode((teCLD_OOSC_InterlockMode)sOnOffConfigServerCluster.eInterlockMode);
                    interlockBuddy->setInterlockState(getState());
                    buttonHandler.resetButtonStateMachine();
                }
                break;

            default:
                // Tests will expect button state machine go IDLE when any of the settings are changed
                buttonHandler.resetButtonStateMachine();
                break;
        }
    }

    // Store received values into PDM
    saveButtonsConfiguration();
}

teZCL_CommandStatus SwitchEndpoint::handleCheckAttributeRange(tsZCL_CallBackEvent *psEvent)
{
    uint16 attribute = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;
    uint16 cluster = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;

    // Prevent switching client only endpoint to a server mode
    if(cluster == GENERAL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION && attribute == E_CLD_OOSC_ATTR_ID_SWITCH_OPERATION_MODE)
    {
        uint8 value = *(uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData;
        if(clientOnly && value != E_CLD_OOSC_OPERATION_MODE_CLIENT)
            return E_ZCL_CMDS_INVALID_VALUE;
    }

    // By default we do not perform attribute value validation
    return E_ZCL_CMDS_SUCCESS;
}

void SwitchEndpoint::handleReportingConfigureRequest(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterID = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    tsZCL_AttributeReportingConfigurationRecord * record = &psEvent->uMessage.sAttributeReportingConfigurationRecord;
    addReportConfiguration(clusterID, record);
}

void SwitchEndpoint::setInterlockMode(teCLD_OOSC_InterlockMode mode)
{
    sOnOffConfigServerCluster.eInterlockMode = mode;
}

void SwitchEndpoint::setInterlockState(bool buddyState)
{
    // In mutual exclusion mode prevent both endpoints to be ON
    if(sOnOffConfigServerCluster.eInterlockMode == E_CLD_OOSC_INTERLOCK_MODE_MUTEX)
        // Always call doStateChange() to prevent tests go out of sync
        doStateChange(getState() && !buddyState, true);

    // In opposite mode endpoints always have different states
    if(sOnOffConfigServerCluster.eInterlockMode == E_CLD_OOSC_INTERLOCK_MODE_OPPOSITE)
        doStateChange(!buddyState, true);
}

bool SwitchEndpoint::runsInServerMode() const
{
    if(clientOnly)
        return false;

    bool serverMode = (sOnOffConfigServerCluster.eOperationMode == E_CLD_OOSC_OPERATION_MODE_SERVER);
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: ServerMode=%d (mode=%d)\n", getEndpointId(), serverMode, sOnOffConfigServerCluster.eOperationMode);
    return serverMode;
}

void SwitchEndpoint::handleDeviceJoin()
{
    // Force resetting the LED
    doStateChange(getState());
}

void SwitchEndpoint::handleDeviceLeave()
{
    // Force resetting the LED
    doStateChange(getState());
}

void SwitchEndpoint::initReportingConfigurations()
{
    memset(reportConfigurations, 0, sizeof(reportConfigurations));
    saveReportingConfigurations();
}

void SwitchEndpoint::saveReportingConfigurations()
{
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Save reporting configuration\n", getEndpointId());
    PDM_eSaveRecordData(getPdmIdForEndpoint(getEndpointId(), PARAM_ID_REPORTING_CONFIG),
                        reportConfigurations,
                        sizeof(reportConfigurations));
}

void SwitchEndpoint::restoreReportingConfigurations()
{
    // Read values from PDM
    uint16 readBytes;
    PDM_teStatus status = PDM_eReadDataFromRecord(getPdmIdForEndpoint(getEndpointId(), PARAM_ID_REPORTING_CONFIG),
                                                  reportConfigurations,
                                                  sizeof(reportConfigurations),
                                                  &readBytes);

    // During the first run it may happen that there is no record in PDM, and we need to initialize it
    if(status != PDM_E_STATUS_OK)
    {
        DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: No PDM record for reporting configuration. Create a new one. status=%d\n", getEndpointId(), status);
        initReportingConfigurations();
        return;
    }

    // Iterate over the loaded array and re-register all reporting configurations
    for(int i = 0; i < ZCL_NUMBER_OF_REPORTS; i++)
    {
        if(reportConfigurations[i].clusterID != 0)
        {
            DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Restore reporting configuration: ClusterID=%04x, AttrID=%04x, Min=%d, Max=%d, Timeout=%d\n",
                        getEndpointId(),
                        reportConfigurations[i].clusterID,
                        reportConfigurations[i].record.u16AttributeEnum,
                        reportConfigurations[i].record.u16MinimumReportingInterval,
                        reportConfigurations[i].record.u16MaximumReportingInterval,
                        reportConfigurations[i].record.u16TimeoutPeriodField);

            eZCL_SetReportableFlag( getEndpointId(), 
                                    reportConfigurations[i].clusterID, 
                                    TRUE, 
                                    FALSE, 
                                    reportConfigurations[i].record.u16AttributeEnum);
            eZCL_CreateLocalReport( getEndpointId(), 
                                    reportConfigurations[i].clusterID, 
                                    0, 
                                    TRUE, 
                                    &reportConfigurations[i].record);
        }
    }
}

uint8 SwitchEndpoint::findReportingConfiguration(uint16 clusterID, uint16 attributeID)
{
    for(int i = 0; i < ZCL_NUMBER_OF_REPORTS; i++)
    {
        // Check if we found an empty slot
        if(reportConfigurations[i].clusterID == 0)
            return i;

        // Search for a record that match passed parameters
        if(reportConfigurations[i].clusterID == clusterID && reportConfigurations[i].record.u16AttributeEnum == attributeID)
            return i;
    }

    // We have not found a record, and there is no empty slot to store a new one
    return ZCL_NUMBER_OF_REPORTS;
}

void SwitchEndpoint::addReportConfiguration(uint16 clusterID, tsZCL_AttributeReportingConfigurationRecord * record)
{
    uint8 index = findReportingConfiguration(clusterID, record->u16AttributeEnum);
    if(index == ZCL_NUMBER_OF_REPORTS)
    {
        DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Warning: No space for new reporting configuration\n", getEndpointId());
        return;
    }

    // Store the new configuration
    reportConfigurations[index].clusterID = clusterID;
    reportConfigurations[index].record = *record;
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Store reporting configuration record at index %d\n", getEndpointId(), index);
    saveReportingConfigurations();
}

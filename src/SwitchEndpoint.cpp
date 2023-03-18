#include "SwitchEndpoint.h"

#include "DumpFunctions.h"
#include "EndpointManager.h"
#include "ZigbeeUtils.h"
#include "ZigbeeDevice.h"
#include "ButtonsTask.h"
#include "PdmIds.h"

extern "C"
{
    #include "dbg.h"
    #include "string.h"
    #include "zcl_customcommand.h"
    #include "PDM.h"
}


SwitchEndpoint::SwitchEndpoint()
{
}

void SwitchEndpoint::setPins(uint8 ledTimer, uint32 pinMask)
{
    ledPin.init(ledTimer);

    ButtonsTask::getInstance()->registerHandler(pinMask, &buttonHandler);
}

void SwitchEndpoint::registerServerCluster()
{
    // Initialize On/Off server cluser
    teZCL_Status status = eCLD_OnOffCreateOnOff(&sClusterInstance.sOnOffServer,
                                                TRUE,                               // Server
                                                &sCLD_OnOff,
                                                &sOnOffServerCluster,
                                                &au8OnOffAttributeControlBits[0],
                                                &sOnOffServerCustomDataStructure);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerClientCluster()
{
    // Initialize On/Off client cluser
    teZCL_Status status = eCLD_OnOffCreateOnOff(&sClusterInstance.sOnOffClient,
                                                FALSE,                              // Client
                                                &sCLD_OnOff,
                                                &sOnOffClientCluster,
                                                &au8OnOffAttributeControlBits[0],
                                                NULL);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff client cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerOnOffConfigServerCluster()
{
    // Initialize On/Off config server cluser
    teZCL_Status status = eCLD_OOSCCreateOnOffSwitchConfig(&sClusterInstance.sOnOffConfigServer,
                                                           TRUE,                              // Server
                                                           &sCLD_OOSC,
                                                           &sOnOffConfigServerCluster,
                                                           &au8OOSCAttributeControlBits[0]);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create OnOff config server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerMultistateInputServerCluster()
{
    // Initialize Multistate Input server cluser
    teZCL_Status status = eCLD_MultistateInputBasicCreateMultistateInputBasic(
                &sClusterInstance.sMultistateInputServer,
                TRUE,                              // Server
                &sCLD_MultistateInputBasic,
                &sMultistateInputServerCluster,
                &au8MultistateInputBasicAttributeControlBits[0]);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create Multistate Input server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerLevelControlClientCluster()
{
    // Initialize Level Control client cluser
    teZCL_Status status = eCLD_LevelControlCreateLevelControl(&sClusterInstance.sLevelControlServer,
                                                              TRUE,                              // Server
                                                              &sCLD_LevelControl,
                                                              &sLevelControlServerCluster,
                                                              &au8LevelControlAttributeControlBits[0],
                                                              &sLevelControlServerCustomDataStructure);

    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Failed to create Level Control server cluster instance. status=%d\n", status);
}

void SwitchEndpoint::registerEndpoint()
{
    // Initialize endpoint structure
    sEndPoint.u8EndPointNumber = getEndpointId();
    sEndPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    sEndPoint.u16ProfileEnum = HA_PROFILE_ID;
    sEndPoint.bIsManufacturerSpecificProfile = FALSE;
    sEndPoint.u16NumberOfClusters = sizeof(OnOffClusterInstances) / sizeof(tsZCL_ClusterInstance);
    sEndPoint.psClusterInstance = (tsZCL_ClusterInstance*)&sClusterInstance;
    sEndPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    sEndPoint.pCallBackFunctions = &EndpointManager::handleZclEvent;

    // Register the endpoint with all the clusters in it
    teZCL_Status status = eZCL_Register(&sEndPoint);
    DBG_vPrintf(TRUE, "SwitchEndpoint::init(): Register Switch Endpoint. status=%d\n", status);
}

void SwitchEndpoint::restoreConfiguration()
{
    // Read values from PDM
    uint16 readBytes;
    PDM_eReadDataFromRecord(getPdmIdForEndpoint(getEndpointId(), 0),
                            &sOnOffConfigServerCluster,
                            sizeof(sOnOffConfigServerCluster),
                            &readBytes);

    // Configure buttons state machine with read values
    buttonHandler.setSwitchMode((SwitchMode)sOnOffConfigServerCluster.eSwitchMode);
    buttonHandler.setRelayMode((RelayMode)sOnOffConfigServerCluster.eRelayMode);
    buttonHandler.setMaxPause(sOnOffConfigServerCluster.iMaxPause);
    buttonHandler.setMinLongPress(sOnOffConfigServerCluster.iMinLongPress);
}

void SwitchEndpoint::saveConfiguration()
{
    PDM_eSaveRecordData(getPdmIdForEndpoint(getEndpointId(), 0),
                        &sOnOffConfigServerCluster,
                        sizeof(sOnOffConfigServerCluster));
}

void SwitchEndpoint::init()
{
    // Register all clusters and endpoint itself
    registerServerCluster();
    registerClientCluster();
    registerOnOffConfigServerCluster();
    registerMultistateInputServerCluster();
    registerLevelControlClientCluster();
    registerEndpoint();

    // Restore previous configuration from PDM
    restoreConfiguration();

    // Initialize LED
    ledPin.setLevel(0);

    // Let button handler know about this Endpoint instanct so that it can properly report new states
    buttonHandler.setEndpoint(this);
}

bool SwitchEndpoint::getState() const
{
    if(runsInServerMode())
        return sOnOffServerCluster.bOnOff;

    return false;
}

void SwitchEndpoint::switchOn()
{
    bool newValue = true;

    // Invert the value in inverse mode
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_S2OFF_S1ON)
        newValue = false;

    doStateChange(newValue);
    reportStateChange();
}

void SwitchEndpoint::switchOff()
{
    bool newValue = false;

    // Invert the value in inverse mode
    if(sOnOffConfigServerCluster.eSwitchActions == E_CLD_OOSC_ACTION_S2OFF_S1ON)
        newValue = true;

    doStateChange(newValue);
    reportStateChange();
}

void SwitchEndpoint::toggle()
{
    doStateChange(!getState());
    reportStateChange();
}

void SwitchEndpoint::doStateChange(bool state)
{
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: do state change %d\n", getEndpointId(), state);

    if(runsInServerMode())
    {
        sOnOffServerCluster.bOnOff = state ? TRUE : FALSE;
        ledPin.setLevel(state ? 255 : 0);
    }
}

void SwitchEndpoint::reportState()
{
    // Destination address - 0x0000 (coordinator)
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT;

    // Send the report
    DBG_vPrintf(TRUE, "Reporting attribute EP=%d value=%d... ", getEndpointId(), sOnOffServerCluster.bOnOff);
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

void SwitchEndpoint::sendCommandToBoundDevices()
{
    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND;

    // Send the toggle command
    uint8 sequenceNo;
    teZCL_Status status = eCLD_OnOffCommandSend(getEndpointId(),
                                   1,
                                   &addr,
                                   &sequenceNo,
                                   E_CLD_ONOFF_CMD_TOGGLE);
    DBG_vPrintf(TRUE, "Sending On/Off command status: %02x\n", status);
}

void SwitchEndpoint::reportLongPress(bool pressed)
{
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

    default:
        break;
    }
}

void SwitchEndpoint::sendLevelControlMoveCommand(bool up)
{
    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND;

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
                                                                  TRUE,
                                                                  &payload);
    DBG_vPrintf(TRUE, "Sending Level Control Move command status: %02x\n", status);
}

void SwitchEndpoint::sendLevelControlStopCommand()
{
    // Destination address does not matter - we will send to all bound devices
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_BOUND;

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
                                                                  TRUE,
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

void SwitchEndpoint::reportStateChange()
{
    if(!ZigbeeDevice::getInstance()->isJoined())
    {
        DBG_vPrintf(TRUE, "Device has not yet joined the network. Ignore reporting the change.\n");
        return;
    }

    if(runsInServerMode())
        reportState();
    else
        sendCommandToBoundDevices();
}

void SwitchEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 u8CommandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Cluster update message ClusterID=%04x evtType=%02x this=%08x Cmd=%02x\n",
                psEvent->u8EndPoint,
                u16ClusterId,
                psEvent->eEventType,
                this,
                u8CommandId);

    if(u16ClusterId == GENERAL_CLUSTER_ID_ONOFF)
        doStateChange(getState());
    if(u16ClusterId == GENERAL_CLUSTER_ID_LEVEL_CONTROL)
    {
        uint8 level = sLevelControlServerCluster.u8CurrentLevel;
        DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: do level change %d (ledPin=%02x)\n", getEndpointId(), level, ledPin.timerId);
        ledPin.setLevel(level);
    }
}

void SwitchEndpoint::handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    uint16 attrId = psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum;

    // Update buttons state machine with received value
    if(clusterId == GENERAL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION)
    {
        if(attrId == E_CLD_OOSC_ATTR_ID_SWITCH_MODE)
            buttonHandler.setSwitchMode((SwitchMode)sOnOffConfigServerCluster.eSwitchMode);

        if(attrId == E_CLD_OOSC_ATTR_ID_SWITCH_RELAY_MODE)
            buttonHandler.setRelayMode((RelayMode)sOnOffConfigServerCluster.eRelayMode);

        if(attrId == E_CLD_OOSC_ATTR_ID_SWITCH_MAX_PAUSE)
            buttonHandler.setMaxPause(sOnOffConfigServerCluster.iMaxPause);

        if(attrId == E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_DUR)
            buttonHandler.setMinLongPress(sOnOffConfigServerCluster.iMinLongPress);
    }

    // Store received values into PDM
    saveConfiguration();
}

bool SwitchEndpoint::runsInServerMode() const
{
    return !hasBindings(getEndpointId(), GENERAL_CLUSTER_ID_ONOFF);
}

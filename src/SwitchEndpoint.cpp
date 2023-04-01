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

void SwitchEndpoint::registerLevelControlServerCluster()
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
    registerLevelControlServerCluster();
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
        DBG_vPrintf(TRUE, "   CUR: OnOff=%d Lvl=%d\n", sOnOffServerCluster.bOnOff, sLevelControlServerCluster.u8CurrentLevel);
        sOnOffServerCluster.bOnOff = state ? TRUE : FALSE;
        DBG_vPrintf(TRUE, "   NEW: OnOff=%d Lvl=%d\n", sOnOffServerCluster.bOnOff, sLevelControlServerCluster.u8CurrentLevel);
        //ledPin.setLevel(state ? 255 : 0);
    }
}

void SwitchEndpoint::doLevelChange(uint8 level)
{
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: do level change %d\n", getEndpointId(), level);

    if(runsInServerMode())
    {
        static const uint8 level2pwm[256] = {
            0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03,
            0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06,
            0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09,
            0x09, 0x0a, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d,
            0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11,
            0x11, 0x11, 0x12, 0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15,
            0x16, 0x16, 0x16, 0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b,
            0x1b, 0x1c, 0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1e, 0x1e, 0x1f, 0x1f, 0x20, 0x20, 0x21, 0x21, 0x22,
            0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x26, 0x26, 0x27, 0x28, 0x28, 0x29, 0x29, 0x2a,
            0x2a, 0x2b, 0x2c, 0x2c, 0x2d, 0x2d, 0x2e, 0x2f, 0x2f, 0x30, 0x31, 0x31, 0x32, 0x33, 0x34, 0x34,
            0x35, 0x36, 0x37, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x42,
            0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52, 0x53, 0x55,
            0x56, 0x57, 0x59, 0x5a, 0x5c, 0x5d, 0x5f, 0x60, 0x62, 0x64, 0x65, 0x67, 0x69, 0x6a, 0x6c, 0x6e,
            0x70, 0x72, 0x74, 0x76, 0x78, 0x7a, 0x7c, 0x7e, 0x80, 0x82, 0x85, 0x87, 0x89, 0x8c, 0x8e, 0x91,
            0x93, 0x96, 0x98, 0x9b, 0x9e, 0xa1, 0xa4, 0xa7, 0xaa, 0xad, 0xb0, 0xb3, 0xb6, 0xba, 0xbd, 0xc1,
            0xc4, 0xc8, 0xcc, 0xcf, 0xd3, 0xd7, 0xdb, 0xdf, 0xe4, 0xe8, 0xec, 0xf1, 0xf6, 0xfa, 0xff, 0xff
        };

        uint8 pwm = sOnOffServerCluster.bOnOff ? level2pwm[level] : 0;
        ledPin.setLevel(pwm);

        DBG_vPrintf(TRUE, "   CUR: OnOff=%d Lvl=%d Pwm=%d\n", sOnOffServerCluster.bOnOff, sLevelControlServerCluster.u8CurrentLevel, pwm);
        sLevelControlServerCluster.u8CurrentLevel = level;
        DBG_vPrintf(TRUE, "   MEW: OnOff=%d Lvl=%d Pwm=%d\n", sOnOffServerCluster.bOnOff, sLevelControlServerCluster.u8CurrentLevel, pwm);
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

void SwitchEndpoint::handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 u8CommandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Cluster custom message ClusterID=%04x Cmd=%02x\n",
                psEvent->u8EndPoint,
                u16ClusterId,
                u8CommandId);

//    if(u16ClusterId == GENERAL_CLUSTER_ID_ONOFF)
//        doStateChange(getState());
}

void SwitchEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;
    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Cluster update message ClusterID=%04x\n",
                psEvent->u8EndPoint,
                u16ClusterId);

    if(u16ClusterId == GENERAL_CLUSTER_ID_ONOFF)
        doStateChange(getState());
    if(u16ClusterId == GENERAL_CLUSTER_ID_LEVEL_CONTROL)
        doLevelChange(sLevelControlServerCluster.u8CurrentLevel);
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

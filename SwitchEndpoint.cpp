#include "SwitchEndpoint.h"

#include "DumpFunctions.h"
#include "EndpointManager.h"

extern "C"
{
    #include "OnOff.h"
    #include "dbg.h"
    #include "string.h"
}

SwitchEndpoint::SwitchEndpoint()
{
}

void SwitchEndpoint::init()
{
    // Initialize the endpoint
    DBG_vPrintf(TRUE, "SwitchEndpoint::init(): register On/Off endpoint #%d...  ", getEndpointId());
    teZCL_Status status = eZLO_RegisterOnOffLightEndPoint(getEndpointId(), &EndpointManager::handleZclEvent, &sSwitch);
    DBG_vPrintf(TRUE, "eApp_ZCL_RegisterEndpoint() status %d\n", status);

    // Initialize blinking
    // Note: this blinking task represents a relay that would be tied with this switch. That is why blinkTask
    // is a property of SwitchEndpoint, and not the global task object
    // TODO: restore previous blink mode from PDM
    blinkTask.setBlinkMode(false);
}

bool SwitchEndpoint::getState() const
{
    return sSwitch.sOnOffServerCluster.bOnOff;
}

void SwitchEndpoint::switchOn()
{
    doStateChange(true);
    reportStateChange();
}

void SwitchEndpoint::switchOff()
{
    doStateChange(false);
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

    sSwitch.sOnOffServerCluster.bOnOff = state ? TRUE : FALSE;

    blinkTask.setBlinkMode(state);
}

void SwitchEndpoint::reportStateChange()
{
    // Destination address - 0x0000 (coordinator)
    tsZCL_Address addr;
    addr.uAddress.u16DestinationAddress = 0x0000;
    addr.eAddressMode = E_ZCL_AM_SHORT;

    DBG_vPrintf(TRUE, "Reporting attribute EP=%d value=%d... ", getEndpointId(), sSwitch.sOnOffServerCluster.bOnOff);
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

void SwitchEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 u8CommandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "SwitchEndpoint EP=%d: Cluster update message ClusterID=%04x Cmd=%02x\n",
                psEvent->u8EndPoint,
                u16ClusterId,
                u8CommandId);

    doStateChange(getState());
}

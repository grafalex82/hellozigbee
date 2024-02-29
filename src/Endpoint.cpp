#include "Endpoint.h"
#include "DumpFunctions.h"

extern "C"
{
    #include "dbg.h"
}

Endpoint::Endpoint()
{
    endpointId = 0;
}

void Endpoint::setEndpointId(uint8 id)
{
    endpointId = id;
}

uint8 Endpoint::getEndpointId() const
{
    return endpointId;
}

void Endpoint::handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default custom cluster event handler for event type (%d)\n", psEvent->eEventType);
}

void Endpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default cluster update handler for event type (%d)\n", psEvent->eEventType);
}

teZCL_CommandStatus Endpoint::handleReadAttribute(tsZCL_CallBackEvent *psEvent)
{
    // By default we do not perform specific attribute read handling
    return E_ZCL_CMDS_SUCCESS;
}

void Endpoint::handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default write attribute handler\n");
}

teZCL_CommandStatus Endpoint::handleCheckAttributeRange(tsZCL_CallBackEvent *psEvent)
{
    // By default we do not perform attribute value validation
    return E_ZCL_CMDS_SUCCESS;
}

void Endpoint::handleReportingConfigureRequest(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default reporting configure handler\n");
}

void Endpoint::handleZclEvent(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_READ_REQUEST:
            psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus = handleReadAttribute(psEvent);
            vDumpZclReadRequest(psEvent);
            break;

        case E_ZCL_CBET_WRITE_INDIVIDUAL_ATTRIBUTE:
            vDumpZclWriteAttributeRequest(psEvent);
            handleWriteAttributeCompleted(psEvent);
            break;

        case E_ZCL_CBET_WRITE_ATTRIBUTES:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Write attributes completed\n");
            break;

        case E_ZCL_CBET_CHECK_ATTRIBUTE_RANGE:
            psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus = handleCheckAttributeRange(psEvent);
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Check attribute %04x on cluster %04x range status %d\n", 
                        psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum,
                        psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum,
                        psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus);
            break;

        case E_ZCL_CBET_UNHANDLED_EVENT:
        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: DEFAULT_RESPONSE received. No action\n");
            break;

        case E_ZCL_CBET_ERROR:
        case E_ZCL_CBET_TIMER:
        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: No action (event type %d)\n", psEvent->eEventType);
            break;

        case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Read Attrib Rsp %d %02x\n", 
                        psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                        *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            handleCustomClusterEvent(psEvent);
            break;

        case E_ZCL_CBET_CLUSTER_UPDATE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Cluster %04x update\n", psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum);
            handleClusterUpdate(psEvent);
            break;

        case E_ZCL_CBET_REPORT_INDIVIDUAL_ATTRIBUTES_CONFIGURE:
            vDumpAttributeReportingConfigureRequest(psEvent);
            if(psEvent->eZCL_Status == E_ZCL_SUCCESS)
                handleReportingConfigureRequest(psEvent);
            break;

        case E_ZCL_CBET_REPORT_ATTRIBUTES_CONFIGURE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Attributes reporting request has been processed\n");
            break;

        default:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
            break;
    }
}

void Endpoint::handleDeviceJoin()
{
    // Nothing to do
}

void Endpoint::handleDeviceLeave()
{
    // Nothing to do
}

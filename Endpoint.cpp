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

void Endpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default cluster update handler for event type (%d)\n", psEvent->eEventType);
}

void Endpoint::handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "Endpoint: Warning: using default write attribute handler\n");
}

void Endpoint::handleZclEvent(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_READ_REQUEST:
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
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Check attribute %04x range. No action\n", psEvent->uMessage.sIndividualAttributeResponse.u16AttributeEnum);
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
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Read Attrib Rsp %d %02x\n", psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
        case E_ZCL_CBET_CLUSTER_UPDATE:
            handleClusterUpdate(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
            break;
    }
}

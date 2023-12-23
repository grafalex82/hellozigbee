#include "EndpointManager.h"
#include "Endpoint.h"
#include <string.h>

extern "C"
{
    #include "dbg.h"
    #include "zcl.h"

    // Local configuration and generated files
    #include "pdum_gen.h"
}

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {

        case E_ZCL_CBET_UNHANDLED_EVENT:
            DBG_vPrintf(TRUE, "ZCL General Callback: Unhandled Event\n");
            break;

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL General Callback: Read attributes response\n");
            break;

        case E_ZCL_CBET_READ_REQUEST:
            DBG_vPrintf(TRUE, "ZCL General Callback: Read request\n");
            break;

        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL General Callback: Default response\n");
            break;

        case E_ZCL_CBET_ERROR:
            DBG_vPrintf(TRUE, "ZCL General Callback: Error\n");
            break;

        case E_ZCL_CBET_TIMER:
            break;

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRUE, "ZCL General Callback: ZigBee\n");
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRUE, "ZCL General Callback: Custom\n");
            break;

        default:
            DBG_vPrintf(TRUE, "ZCL General Callback: Invalid event type (%d) in APP_ZCL_cbGeneralCallback\n", psEvent->eEventType);
            break;
    }
}

EndpointManager::EndpointManager()
{
    // Make sure all endpoint pointers are uninitialized
    memset(registry, 0, sizeof(Endpoint*) * (ZCL_NUMBER_OF_ENDPOINTS+1));

    // Initialize ZCL
    DBG_vPrintf(TRUE, "EndpointManager::EndpointManager(): init Zigbee Class Library (ZCL)...  ");
    ZPS_teStatus status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    DBG_vPrintf(TRUE, "eZCL_Initialise() status %d\n", status);
}

EndpointManager * EndpointManager::getInstance()
{
    static EndpointManager instance;
    return &instance;
}

void EndpointManager::registerEndpoint(uint8 id, Endpoint * endpoint)
{
    registry[id] = endpoint;
    endpoint->setEndpointId(id);
    endpoint->init();
}

void EndpointManager::handleZclEventInt(tsZCL_CallBackEvent *psEvent)
{
    uint8 ep = psEvent->u8EndPoint;

    if(ep > 0 && ep <= ZCL_NUMBER_OF_ENDPOINTS)
        registry[ep]->handleZclEvent(psEvent);
    else
        DBG_vPrintf(TRUE, "EndpointManager::handleZclEvent(): Invalid endpoint number %d\n", ep);
}

void EndpointManager::handleZclEvent(tsZCL_CallBackEvent *psEvent)
{
    EndpointManager::getInstance()->handleZclEventInt(psEvent);
}

void EndpointManager::handleDeviceJoin()
{
    for(uint8 ep = 1; ep <= ZCL_NUMBER_OF_ENDPOINTS; ep++)
        registry[ep]->handleDeviceJoin();
}

void EndpointManager::handleDeviceLeave()
{
    for(uint8 ep = 1; ep <= ZCL_NUMBER_OF_ENDPOINTS; ep++)
        registry[ep]->handleDeviceJoin();
}
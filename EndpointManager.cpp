#include "EndpointManager.h"
#include "Endpoint.h"
#include <string.h>

extern "C"
{
    #include "dbg.h"
}

EndpointManager::EndpointManager()
{
    DBG_vPrintf(TRUE, "Endpoint Manager constructor\n");
    memset(registry, 0, sizeof(Endpoint*) * ZCL_NUMBER_OF_ENDPOINTS);
}

EndpointManager * EndpointManager::getInstance()
{
    DBG_vPrintf(TRUE, "About to create Endpoint Manager\n");
    static EndpointManager instance;
    DBG_vPrintf(TRUE, "Returning Endpoint Manager\n");
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
    registry[ep]->handleZclEvent(psEvent);
}

void EndpointManager::handleZclEvent(tsZCL_CallBackEvent *psEvent)
{
    EndpointManager::getInstance()->handleZclEventInt(psEvent);
}

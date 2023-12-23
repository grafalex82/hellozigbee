#ifndef ENDPOINTMANAGER_H
#define ENDPOINTMANAGER_H

extern "C"
{
    #include "zcl.h"
    #include "zcl_options.h"
}

class Endpoint;

class EndpointManager
{
private:
    EndpointManager();

    Endpoint * registry[ZCL_NUMBER_OF_ENDPOINTS+1];

public:
    static EndpointManager * getInstance();

    void registerEndpoint(uint8 id, Endpoint * endpoint);

    static void handleZclEvent(tsZCL_CallBackEvent *psEvent);
    void handleDeviceJoin();
    void handleDeviceLeave();

protected:
    void handleZclEventInt(tsZCL_CallBackEvent *psEvent);
};



#endif // ENDPOINTMANAGER_H

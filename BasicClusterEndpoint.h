#ifndef BASICCLUSTERENDPOINT_H
#define BASICCLUSTERENDPOINT_H

#include "Endpoint.h"

extern "C"
{
    #include "zcl.h"
    #include "Basic.h"
}

// List of cluster instances (descriptor objects) that are included into an Endpoint
struct BasicClusterInstances
{
    // All devices have basic mandatory clusters
    #if (defined CLD_BASIC) && (defined BASIC_SERVER)
        tsZCL_ClusterInstance sBasicServer;
    #endif

    // Zigbee device may have also OTA optional clusters for the client
    #if (defined CLD_OTA) && (defined OTA_CLIENT)
        tsZCL_ClusterInstance sOTAClient;
    #endif
} __attribute__ ((aligned(4)));


// Endpoint low level structure
struct BasicClusterDevice
{
    tsZCL_EndPointDefinition endPoint;

    // Cluster instances
    BasicClusterInstances clusterInstances;

    // Value storage for endpoint's clusters
    #if (defined CLD_BASIC) && (defined BASIC_SERVER)
        // Basic Cluster - Server
        tsCLD_Basic sBasicServerCluster;
    #endif

    // On Off light device 2 optional clusters for the client
    #if (defined CLD_OTA) && (defined OTA_CLIENT)
        // OTA cluster - Client
        tsCLD_AS_Ota sCLD_OTA;
        tsOTA_Common sCLD_OTA_CustomDataStruct;
    #endif
};

class BasicClusterEndpoint : public Endpoint
{
    BasicClusterDevice deviceObject;

public:
    BasicClusterEndpoint();

    virtual void init();
};

#endif // BASICCLUSTERENDPOINT_H

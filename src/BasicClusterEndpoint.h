#ifndef BASICCLUSTERENDPOINT_H
#define BASICCLUSTERENDPOINT_H

#include "Endpoint.h"
#include "OTAHandlers.h"

extern "C"
{
    #include "zcl.h"
    #include "Basic.h"
    #include "Identify.h"
    #include "DeviceTemperatureConfiguration.h"
#ifdef SUPPORTS_POWER_METERING
    #include "ElectricalMeasurement.h"
#endif
#ifdef SUPPORTS_POWER_METERING
    #include "SimpleMetering.h"
#endif
}

// List of cluster instances (descriptor objects) that are included into an Endpoint
struct BasicClusterInstances
{
    // All devices have basic mandatory clusters
    tsZCL_ClusterInstance sBasicServer;

    // Zigbee device may have also OTA optional clusters for the client
    tsZCL_ClusterInstance sOTAClient;

    // Identify cluster (whole device handler)
    tsZCL_ClusterInstance sIdentifyServer;

    // The device will report its temperature over the Device Temperature Configuration cluster
    tsZCL_ClusterInstance sDeviceTemperatureServer;

#ifdef SUPPORTS_POWER_METERING
    // Boards with a metering IC report power/voltage/current over the Electrical Measurement cluster
    tsZCL_ClusterInstance sElectricalMeasurementServer;
#endif

#ifdef SUPPORTS_POWER_METERING
    // ... and cumulative energy over the Simple Metering cluster
    tsZCL_ClusterInstance sSimpleMeteringServer;
#endif
} __attribute__ ((aligned(4)));

class BasicClusterEndpoint : public Endpoint
{
    tsZCL_EndPointDefinition endPoint;

    // Cluster instances
    BasicClusterInstances clusterInstances;

    // Value storage for endpoint's clusters
    tsCLD_Basic sBasicServerCluster;
    tsCLD_Identify sIdentifyServerCluster;
    tsCLD_IdentifyCustomDataStructure sIdentifyClusterData;
    tsCLD_DeviceTemperatureConfiguration sDeviceTemperatureServerCluster;
#ifdef SUPPORTS_POWER_METERING
    tsCLD_ElectricalMeasurement sElectricalMeasurementServerCluster;
#endif
#ifdef SUPPORTS_POWER_METERING
    tsCLD_SimpleMetering sSimpleMeteringServerCluster;
    tsSM_CustomStruct sSimpleMeteringCustomDataStruct;
#endif
    tsCLD_AS_Ota sOTAClientCluster;
    tsOTA_Common sOTACustomDataStruct;

    OTAHandlers otaHandlers;

public:
    BasicClusterEndpoint();

    virtual void init();

protected:
    virtual void registerBasicCluster();
    virtual void registerIdentifyCluster();
    virtual void registerOtaCluster();
    virtual void registerDeviceTemperatureCluster();
#ifdef SUPPORTS_POWER_METERING
    virtual void registerElectricalMeasurementCluster();
#endif
#ifdef SUPPORTS_POWER_METERING
    virtual void registerSimpleMeteringCluster();
#endif
    virtual void registerEndpoint();

    virtual void handleClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual void handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent);
    virtual teZCL_CommandStatus handleReadAttribute(tsZCL_CallBackEvent *psEvent);

    void handleIdentifyClusterEvent(tsZCL_CallBackEvent *psEvent);
    void handleOTAClusterEvent(tsZCL_CallBackEvent *psEvent);
    void handleIdentifyClusterUpdate(tsZCL_CallBackEvent *psEvent);
    void handleOTAClusterUpdate(tsZCL_CallBackEvent *psEvent);

    void readDeviceTemperature();

public:
#ifdef SUPPORTS_POWER_METERING
    // Called by EnergyMeterTask every sampling window - keeps the cluster
    // structs fresh for both explicit reads and the attribute reporting engine
    void updateMeteringAttributes();
#endif
};

#endif // BASICCLUSTERENDPOINT_H

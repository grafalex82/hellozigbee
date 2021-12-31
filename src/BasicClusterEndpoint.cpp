#include "BasicClusterEndpoint.h"
#include "EndpointManager.h"

extern "C"
{
    #include "dbg.h"
    #include "string.h"
}

BasicClusterEndpoint::BasicClusterEndpoint()
{

}

void BasicClusterEndpoint::registerBasicCluster()
{
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_BasicCreateBasic(&clusterInstances.sBasicServer,
                                                TRUE,
                                                &sCLD_Basic,
                                                &sBasicServerCluster,
                                                &au8BasicClusterAttributeControlBits[0]);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to create Basic Cluster instance. status=%d\n", status);
}

void BasicClusterEndpoint::registerOtaCluster()
{
    // Create an instance of an OTA cluster as a client */
    teZCL_Status status = eOTA_Create(&clusterInstances.sOTAClient,
                                      FALSE,  /* client */
                                      &sCLD_OTA,
                                      &sOTAClientCluster,  /* cluster definition */
                                      getEndpointId(),
                                      NULL,
                                      &sOTACustomDataStruct);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to create OTA Cluster instance. status=%d\n", status);}

void BasicClusterEndpoint::registerEndpoint()
{
    // Fill in end point details
    endPoint.u8EndPointNumber = getEndpointId();
    endPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    endPoint.u16ProfileEnum = HA_PROFILE_ID;
    endPoint.bIsManufacturerSpecificProfile = FALSE;
    endPoint.u16NumberOfClusters = sizeof(BasicClusterInstances) / sizeof(tsZCL_ClusterInstance);
    endPoint.psClusterInstance = (tsZCL_ClusterInstance*)&clusterInstances;
    endPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    endPoint.pCallBackFunctions = &EndpointManager::handleZclEvent;

    // Register the endpoint with all the clusters above
    teZCL_Status status = eZCL_Register(&endPoint);
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Register Basic Cluster Endpoint. status=%d\n", status);
}

void BasicClusterEndpoint::init()
{
    registerBasicCluster();
    registerOtaCluster();
    registerEndpoint();

    // Fill Basic cluster attributes
    memcpy(sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);
    sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_WALL_SWITCH;

    // Initialize OTA
    otaHandlers.initOTA(getEndpointId());
}

void BasicClusterEndpoint::handleClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    // This function handles only OTA messages
    if(psEvent->uMessage.sClusterCustomMessage.u16ClusterId != OTA_CLUSTER_ID)
        return;

    // Parse and process OTA message
    tsOTA_CallBackMessage *psCallBackMessage = (tsOTA_CallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    otaHandlers.handleOTAMessage(psCallBackMessage);
}


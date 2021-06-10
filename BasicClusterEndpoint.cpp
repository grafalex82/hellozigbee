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

void BasicClusterEndpoint::init()
{
    // Fill in end point details
    deviceObject.endPoint.u8EndPointNumber = getEndpointId();
    deviceObject.endPoint.u16ManufacturerCode = ZCL_MANUFACTURER_CODE;
    deviceObject.endPoint.u16ProfileEnum = HA_PROFILE_ID;
    deviceObject.endPoint.bIsManufacturerSpecificProfile = FALSE;
    deviceObject.endPoint.u16NumberOfClusters = sizeof(BasicClusterInstances) / sizeof(tsZCL_ClusterInstance);
    deviceObject.endPoint.psClusterInstance = (tsZCL_ClusterInstance*)&deviceObject.clusterInstances;
    deviceObject.endPoint.bDisableDefaultResponse = ZCL_DISABLE_DEFAULT_RESPONSES;
    deviceObject.endPoint.pCallBackFunctions = &EndpointManager::handleZclEvent;

    #if (defined CLD_BASIC) && (defined BASIC_SERVER)
        // Create an instance of a basic cluster as a server
        teZCL_Status status = eCLD_BasicCreateBasic(&deviceObject.clusterInstances.sBasicServer,
                                                    TRUE,
                                                    &sCLD_Basic,
                                                    &deviceObject.sBasicServerCluster,
                                                    &au8BasicClusterAttributeControlBits[0]);
        if( status != E_ZCL_SUCCESS)
            DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to create Basic Cluster instance. status=%d\n", status);
    #endif

    #if (defined CLD_OTA) && (defined OTA_CLIENT)
        // Create an instance of an OTA cluster as a client */
        status = eOTA_Create(&deviceObject.clusterInstances.sOTAClient,
                             FALSE,  /* client */
                             &sCLD_OTA,
                             &deviceObject.sCLD_OTA,  /* cluster definition */
                             u8EndPointIdentifier,
                             NULL,
                             &deviceObject.sCLD_OTA_CustomDataStruct);
        if(status != E_ZCL_SUCCESS)
            DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to create OTA Cluster instance. status=%d\n", status);
    #endif

    // Register the endpoint with all the clusters above
    status = eZCL_Register(&deviceObject.endPoint);
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Register Basic Cluster. status=%d\n", status);

    // Fill Basic cluster attributes
    memcpy(deviceObject.sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(deviceObject.sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(deviceObject.sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(deviceObject.sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);
    deviceObject.sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_WALL_SWITCH;
}

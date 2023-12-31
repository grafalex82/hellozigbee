extern "C"
{
    #include "jendefs.h"
    #include "zps_gen.h"
    #include "dbg.h"
    #include "string.h"
}

#include "BasicClusterEndpoint.h"
#include "EndpointManager.h"
#include "LEDTask.h"

BasicClusterEndpoint::BasicClusterEndpoint()
{

}

void BasicClusterEndpoint::registerBasicCluster()
{
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerBasicCluster(): Registering basic cluster\n");
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_BasicCreateBasic(&clusterInstances.sBasicServer,
                                                TRUE,
                                                &sCLD_Basic,
                                                &sBasicServerCluster,
                                                &au8BasicClusterAttributeControlBits[0]);
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerBasicCluster(): Failed to create Basic Cluster instance. Status=%d\n", status);
}

void BasicClusterEndpoint::registerIdentifyCluster()
{
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerIdentifyCluster(): Registering identify cluster\n");
    // Create an instance of a basic cluster as a server
    teZCL_Status status = eCLD_IdentifyCreateIdentify(&clusterInstances.sIdentifyServer,
                                                TRUE,
                                                &sCLD_Identify,
                                                &sIdentifyServerCluster,
                                                &au8IdentifyAttributeControlBits[0],
                                                &sIdentifyClusterData);
    
    if( status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerIdentifyCluster(): Failed to create Identify Cluster instance. Status=%d\n", status);
}

void BasicClusterEndpoint::registerOtaCluster()
{
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerOtaCluster(): Registering ota cluster\n");
    // Create an instance of an OTA cluster as a client */
    teZCL_Status status = eOTA_Create(&clusterInstances.sOTAClient,
                                      FALSE,  /* client */
                                      &sCLD_OTA,
                                      &sOTAClientCluster,  /* cluster definition */
                                      getEndpointId(),
                                      NULL,
                                      &sOTACustomDataStruct);

    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerOtaCluster(): Failed to create OTA Cluster instance. Status=%d\n", status);
}

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
    DBG_vPrintf(TRUE, "BasicClusterEndpoint::registerEndpoint(): Register Basic Cluster Endpoint. Status=%d\n", status);
}

void BasicClusterEndpoint::init()
{
    registerBasicCluster();
    registerIdentifyCluster();
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
    uint16 clusterId = psEvent->psClusterInstance->psClusterDefinition->u16ClusterEnum;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterUpdate(psEvent);
            break;

        case OTA_CLUSTER_ID:
            handleOTAClusterUpdate(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Warning: Unexpected cluster update message ClusterID=%04x\n", clusterId);
            break;
    }
}

void BasicClusterEndpoint::handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    uint16 clusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;

    switch(clusterId)
    {
        case GENERAL_CLUSTER_ID_IDENTIFY:
            handleIdentifyClusterEvent(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Warning: Unexpected custom cluster event ClusterID=%04x\n", 
                        getEndpointId(), clusterId);
            break;
    }
}

void BasicClusterEndpoint::handleIdentifyClusterEvent(tsZCL_CallBackEvent *psEvent)
{
    tsCLD_IdentifyCallBackMessage * msg = (tsCLD_IdentifyCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 commandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Identify cluster command Cmd=%d\n",
                psEvent->u8EndPoint,
                commandId);

    switch(commandId)
    {
        case E_CLD_IDENTIFY_CMD_IDENTIFY:
            LEDTask::getInstance()->triggerEffect(HELLOZIGBEE_BASIC_ENDPOINT, E_CLD_IDENTIFY_EFFECT_BREATHE);
            break;

        case E_CLD_IDENTIFY_CMD_TRIGGER_EFFECT:
            LEDTask::getInstance()->triggerEffect(HELLOZIGBEE_BASIC_ENDPOINT, msg->uMessage.psTriggerEffectRequestPayload->eEffectId);
            break;

        default:
            break;
    }
}

void BasicClusterEndpoint::handleIdentifyClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    zuint16 identifyTime = sIdentifyServerCluster.u16IdentifyTime;
    DBG_vPrintf(TRUE, "BasicClusterEndpoint EP=%d: Identify cluster update event. Identify Time = %d\n",
                psEvent->u8EndPoint, 
                identifyTime);

    if(identifyTime == 0)
        LEDTask::getInstance()->stopEffect();
}

void BasicClusterEndpoint::handleOTAClusterUpdate(tsZCL_CallBackEvent *psEvent)
{
    // Parse and process OTA message
    tsOTA_CallBackMessage *psCallBackMessage = (tsOTA_CallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    otaHandlers.handleOTAMessage(psCallBackMessage);
}

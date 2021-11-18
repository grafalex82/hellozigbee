#include "BasicClusterEndpoint.h"
#include "EndpointManager.h"
#include "PersistedValue.h"
#include "PdmIds.h"

extern "C"
{
    #include "dbg.h"
    #include "string.h"
}

void resetPersistedOTAData(tsOTA_PersistedData * persistedData)
{
    memset(persistedData, 0, sizeof(tsOTA_PersistedData));
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

void BasicClusterEndpoint::initOTA()
{
    // Reset attributes to their default value
    teZCL_Status status = eOTA_UpdateClientAttributes(getEndpointId(), 0);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to create OTA Cluster attributes. status=%d\n", status);

    // Restore previous values
    PersistedValue<tsOTA_PersistedData, PDM_ID_OTA_DATA> sPersistedData;
    sPersistedData.init(resetPersistedOTAData);
    status = eOTA_RestoreClientData( getEndpointId(), &sPersistedData, TRUE);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to restore OTA data. status=%d\n", status);

    // Remap flash memory
    if (u32REG_SysRead(REG_SYS_FLASH_REMAP) & 0xf)
    {
        vREG_SysWrite(REG_SYS_FLASH_REMAP,  0xfedcba98);
        vREG_SysWrite(REG_SYS_FLASH_REMAP2, 0x76543210);
    }

    // Initialize flash memory for storing downloaded firmwares
    tsNvmDefs sNvmDefs;
    sNvmDefs.u32SectorSize = 32*1024; // Sector Size = 32K
    sNvmDefs.u8FlashDeviceType = E_FL_CHIP_INTERNAL;
    vOTA_FlashInit(NULL, &sNvmDefs);

    // Fill some OTA related records for the endpoint
    uint8 au8CAPublicKey[22] = {0};
    uint8 u8StartSector[1] = {8};
    status = eOTA_AllocateEndpointOTASpace(
                            getEndpointId(),
                            u8StartSector,
                            OTA_MAX_IMAGES_PER_ENDPOINT,
                            8,                                 // max sectors per image
                            FALSE,
                            au8CAPublicKey);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "BasicClusterEndpoint::init(): Failed to allocate endpoint OTA space (can be ignored for non-OTA builds). status=%d\n", status);


    // Just dump OTA data
    tsOTA_ImageHeader          sOTAHeader;
    eOTA_GetCurrentOtaHeader(getEndpointId(), FALSE, &sOTAHeader);
    DBG_vPrintf(TRUE, "\n\nCurrent Image Details \n");
    DBG_vPrintf(TRUE, "File ID = 0x%08x\n",sOTAHeader.u32FileIdentifier);
    DBG_vPrintf(TRUE, "Header Ver ID = 0x%04x\n",sOTAHeader.u16HeaderVersion);
    DBG_vPrintf(TRUE, "Header Length ID = 0x%04x\n",sOTAHeader.u16HeaderLength);
    DBG_vPrintf(TRUE, "Header Control Field = 0x%04x\n",sOTAHeader.u16HeaderControlField);
    DBG_vPrintf(TRUE, "Manufac Code = 0x%04x\n",sOTAHeader.u16ManufacturerCode);
    DBG_vPrintf(TRUE, "Image Type = 0x%04x\n",sOTAHeader.u16ImageType);
    DBG_vPrintf(TRUE, "File Ver = 0x%08x\n",sOTAHeader.u32FileVersion);
    DBG_vPrintf(TRUE, "Stack Ver = 0x%04x\n",sOTAHeader.u16StackVersion);
    DBG_vPrintf(TRUE, "Image Len = 0x%08x\n\n\n",sOTAHeader.u32TotalImage);
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
    initOTA();
}

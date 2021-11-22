#include "OTAHandlers.h"
#include "DumpFunctions.h"
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

OTAHandlers::OTAHandlers()
{
    otaEp = 0;
}

void OTAHandlers::initOTA(uint8 ep)
{
    otaEp = ep;

    restoreOTAAttributes();
    initFlash();

    // Just dump current image OTA header and MAC address
    vDumpCurrentImageOTAHeader(otaEp);
    vDumpOverridenMacAddress();
}

void OTAHandlers::restoreOTAAttributes()
{
    // Reset attributes to their default value
    teZCL_Status status = eOTA_UpdateClientAttributes(otaEp, 0);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "OTAHandlers::restoreOTAAttributes(): Failed to create OTA Cluster attributes. status=%d\n", status);

    // Restore previous values
    PersistedValue<tsOTA_PersistedData, PDM_ID_OTA_DATA> sPersistedData;
    sPersistedData.init(resetPersistedOTAData);
    status = eOTA_RestoreClientData(otaEp, &sPersistedData, TRUE);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "OTAHandlers::restoreOTAAttributes(): Failed to restore OTA data. status=%d\n", status);
}

void OTAHandlers::initFlash()
{
    // Fix and streamline possible incorrect or non-contiguous flash remapping
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
    teZCL_Status status = eOTA_AllocateEndpointOTASpace(
                            otaEp,
                            u8StartSector,
                            OTA_MAX_IMAGES_PER_ENDPOINT,
                            8,                                 // max sectors per image
                            FALSE,
                            au8CAPublicKey);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "OTAHandlers::initFlash(): Failed to allocate endpoint OTA space (can be ignored for non-OTA builds). status=%d\n", status);
}

void OTAHandlers::handleOTAMessage(tsOTA_CallBackMessage * pMsg)
{
    DBG_vPrintf(TRUE, "OTA Callback Message: evt=%d\n", pMsg->eEventId);
    DBG_vPrintf(TRUE, "OTA Callback Message: fnPointer=0x%08x\n", pMsg->sPersistedData.u32FunctionPointer);

    switch(pMsg->eEventId)
    {
    case E_CLD_OTA_COMMAND_IMAGE_NOTIFY:
    case E_CLD_OTA_INTERNAL_COMMAND_SAVE_CONTEXT:
    case E_CLD_OTA_INTERNAL_COMMAND_POLL_REQUIRED:
    default:
        DBG_vPrintf(TRUE, "OTA Callback Message: Unknown event type evt=%d\n", pMsg->eEventId);
    }
}

void OTAHandlers::vDumpOverridenMacAddress()
{
    static uint8 au8MacAddress[]  __attribute__ ((section (".ro_mac_address"))) = {
                      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    DBG_vPrintf(TRUE, "MAC Address at address = %08x:  ", au8MacAddress);
    for (int i=0; i<8; i++)
        DBG_vPrintf(TRUE, "%02x ",au8MacAddress[i] );
    DBG_vPrintf(TRUE, "\n\n");
}

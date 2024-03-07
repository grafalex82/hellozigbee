#include "OTAHandlers.h"
#include "DumpFunctions.h"

extern "C"
{
    #include "dbg.h"
    #include "string.h"
}

// Pre-configured Link Key (Use one that match Xiaomi Aqara devices - ZigbeeAlliance09)
uint8 s_au8LnkKeyArray[16] __attribute__ ((section (".ro_se_lnkKey")))
= { 0x5A, 0x69, 0x67, 0x42, 0x65, 0x65, 0x41, 0x6C, 0x6C, 0x69, 0x61, 0x6E, 0x63, 0x65, 0x30, 0x39};


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
    #if TRACE_OTA_DEBUG
        vDumpCurrentImageOTAHeader(otaEp);
        vDumpOverridenMacAddress();
    #endif //TRACE_OTA_DEBUG
}

void OTAHandlers::restoreOTAAttributes()
{
    // Reset attributes to their default value
    teZCL_Status status = eOTA_UpdateClientAttributes(otaEp, 0);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "OTAHandlers::restoreOTAAttributes(): Failed to create OTA Cluster attributes. status=%d\n", status);

    // Restore previous values or reset to zeroes
    sPersistedData.init(resetPersistedOTAData, "OTA Data");

    // Correct retry timer to force retry in 10 seconds
    if((&sPersistedData)->u32RequestBlockRequestTime != 0)
    {
        DBG_vPrintf(TRUE, "OTAHandlers::restoreOTAAttriutes(): Will retry current operation in 10 seconds (old value %d)\n", (&sPersistedData)->u32RequestBlockRequestTime);
        (&sPersistedData)->u32RequestBlockRequestTime = 10;
    }

    status = eOTA_RestoreClientData(otaEp, &sPersistedData, TRUE);
    if(status != E_ZCL_SUCCESS)
        DBG_vPrintf(TRUE, "OTAHandlers::restoreOTAAttributes(): Failed to restore OTA data. status=%d\n", status);
}

void OTAHandlers::initFlash()
{
    DBG_vPrintf(TRUE, "OTAHandlers::initFlash(): init OTA handlers and prepare OTA flash space\n");

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

void OTAHandlers::saveOTAContext(tsOTA_PersistedData * pData)
{
    DBG_vPrintf(TRUE, "Saving OTA Context... ");

    // Store the data
    sPersistedData = *pData;
}

void OTAHandlers::handleOTAMessage(tsOTA_CallBackMessage * pMsg)
{
    vDumpOTAMessage(pMsg);

    switch(pMsg->eEventId)
    {
    case E_CLD_OTA_INTERNAL_COMMAND_SAVE_CONTEXT:
        saveOTAContext(&pMsg->sPersistedData);
        break;
    default:
        break;
    }
}


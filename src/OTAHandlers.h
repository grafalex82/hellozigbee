#ifndef OTAHANDLERS_H
#define OTAHANDLERS_H

#include "PersistedValue.h"
#include "PdmIds.h"

extern "C"
{
    #include "jendefs.h"

    #include "OTA.h"

    #ifndef OTA_H_FIXED
    #define OTA_H_FIXED
    } // Missed '}' in OTA.h
    #endif //OTA_H_FIXED
}

class OTAHandlers
{
    uint8 otaEp;
    PersistedValue<tsOTA_PersistedData, PDM_ID_OTA_DATA> sPersistedData;

public:
    OTAHandlers();

    void initOTA(uint8 ep);
    void handleOTAMessage(tsOTA_CallBackMessage * psCallBackMessage);

private:
    void restoreOTAAttributes();
    void initFlash();
    void saveOTAContext(tsOTA_PersistedData * pData);
};

#endif // OTAHANDLERS_H

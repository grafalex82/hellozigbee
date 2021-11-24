#ifndef OTAHANDLERS_H
#define OTAHANDLERS_H

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

public:
    OTAHandlers();

    void initOTA(uint8 ep);
    void handleOTAMessage(tsOTA_CallBackMessage * psCallBackMessage);

private:
    void restoreOTAAttributes();
    void initFlash();
    void vDumpOverridenMacAddress();
};

#endif // OTAHANDLERS_H

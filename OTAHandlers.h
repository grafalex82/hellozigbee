#ifndef OTAHANDLERS_H
#define OTAHANDLERS_H

extern "C"
{
    #include "jendefs.h"

    #include "OTA.h"
    } // Missing '}' in OTA.h
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
};

#endif // OTAHANDLERS_H

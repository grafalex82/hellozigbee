#ifndef OTAHANDLERS_H
#define OTAHANDLERS_H

extern "C"
{
    #include "jendefs.h"
}

class OTAHandlers
{
    uint8 otaEp;

public:
    OTAHandlers();

    void initOTA(uint8 ep);

private:
    void restoreOTAAttributes();
    void initFlash();
};

#endif // OTAHANDLERS_H

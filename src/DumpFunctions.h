#ifndef DUMPFUNCTIONS_H
#define DUMPFUNCTIONS_H

extern "C"
{
    #include "OTA.h"

    #ifndef OTA_H_FIXED
    #define OTA_H_FIXED
    } // Missed '}' in OTA.h
    #endif //OTA_H_FIXED
}

extern "C"
{
    #include "zcl.h"

    void vDumpZclReadRequest(tsZCL_CallBackEvent *psEvent);
    void vDumpZclWriteAttributeRequest(tsZCL_CallBackEvent *psEvent);
    void vDumpAttributeReportingConfigureRequest(tsZCL_CallBackEvent *psEvent);
    void vDumpAfEvent(ZPS_tsAfEvent* psStackEvent);
    void vDumpNetworkParameters();
    void vDisplayDiscoveredNodes();
    void vDisplayNeighbourTable();
    void vDisplayBindTable();
    void vDisplayGroupsTable();
    void vDisplayAddressMap();

    void vDumpOverridenMacAddress();
    void vDumpCurrentImageOTAHeader(uint8 otaEp);
    void vDumpOTAMessage(tsOTA_CallBackMessage * pMsg);
}


#endif // DUMPFUNCTIONS_H

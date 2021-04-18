#ifndef DUMPFUNCTIONS_H
#define DUMPFUNCTIONS_H

extern "C"
{
    #include "zcl.h"

    void vDumpZclReadRequest(tsZCL_CallBackEvent *psEvent);
    void vDumpAfEvent(ZPS_tsAfEvent* psStackEvent);
    void vDumpNetworkParameters();
}


#endif // DUMPFUNCTIONS_H

#ifndef ENDPOINT_H
#define ENDPOINT_H

extern "C"
{
    #include "zcl.h"
}


class Endpoint
{
    uint8 endpointId;

public:
    Endpoint();

    void setEndpointId(uint8 id);
    uint8 getEndpointId() const;

    virtual void init() = 0;
    virtual void handleZclEvent(tsZCL_CallBackEvent *psEvent);

    virtual void handleDeviceJoin();
    virtual void handleDeviceLeave();

protected:
    virtual void handleCustomClusterEvent(tsZCL_CallBackEvent *psEvent);
    virtual void handleClusterUpdate(tsZCL_CallBackEvent *psEvent);
    virtual teZCL_CommandStatus handleReadAttribute(tsZCL_CallBackEvent *psEvent);
    virtual void handleWriteAttributeCompleted(tsZCL_CallBackEvent *psEvent);
    virtual teZCL_CommandStatus handleCheckAttributeRange(tsZCL_CallBackEvent *psEvent);
    virtual void handleReportingConfigureRequest(tsZCL_CallBackEvent *psEvent);
};

#endif // ENDPOINT_H

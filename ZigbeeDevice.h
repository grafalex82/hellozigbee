#ifndef ZIGBEEDEVICE_H
#define ZIGBEEDEVICE_H

#include "PersistedValue.h"
#include "PdmIds.h"
#include "PollTask.h"
#include "Queue.h"

extern "C"
{
    #include "zps_apl_af.h"
}

typedef enum
{
    NOT_JOINED,
    JOINING,
    JOINED

} JoinStateEnum;

class ZigbeeDevice
{
    PersistedValue<JoinStateEnum, PDM_ID_NODE_STATE> connectionState;
    Queue<BDB_tsZpsAfEvent, 3> bdbEventQueue;
    PollTask pollTask;

public:
    ZigbeeDevice();

    static ZigbeeDevice * getInstance();

    JoinStateEnum getState() const {return connectionState;}
    void setState(JoinStateEnum state) {connectionState = state;}

    void joinNetwork();
    void leaveNetwork();
    void joinOrLeaveNetwork();

protected:
    void handleNetworkJoinAndRejoin();
    void handleLeaveNetwork();
    void handleRejoinFailure();
    void handlePollResponse(ZPS_tsAfPollConfEvent* pEvent);
    void handleZdoBindEvent(ZPS_tsAfZdoBindEvent * pEvent);
    void handleZdoUnbindEvent(ZPS_tsAfZdoUnbindEvent * pEvent);
    void handleZdoDataIndication(ZPS_tsAfEvent * pEvent);
    void handleZdoEvents(ZPS_tsAfEvent* psStackEvent);
    void handleZclEvents(ZPS_tsAfEvent* psStackEvent);
    void handleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent);

public:
    void handleBdbEvent(BDB_tsBdbEvent *psBdbEvent);
};

#endif // ZIGBEEDEVICE_H

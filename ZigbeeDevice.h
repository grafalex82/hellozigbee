#ifndef ZIGBEEDEVICE_H
#define ZIGBEEDEVICE_H

#include "PersistedValue.h"
#include "PdmIds.h"
#include "PollTask.h"

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

public:
    ZigbeeDevice();

    static ZigbeeDevice * getInstance();

    JoinStateEnum getState() const {return connectionState;}
    void setState(JoinStateEnum state) {connectionState = state;}

public:
    void handleBdbEvent(BDB_tsBdbEvent *psBdbEvent);
};

#endif // ZIGBEEDEVICE_H

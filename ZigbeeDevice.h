#ifndef ZIGBEEDEVICE_H
#define ZIGBEEDEVICE_H

#include "PersistedValue.h"
#include "PdmIds.h"

typedef enum
{
    NOT_JOINED,
    JOINING,
    JOINED

} JoinStateEnum;

class ZigbeeDevice
{
public:
    ZigbeeDevice();
};

#endif // ZIGBEEDEVICE_H

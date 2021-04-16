#ifndef CONNECTIONSTATE_H
#define CONNECTIONSTATE_H

#include "PersistedValue.h"

#define PWM_ID_NODE_STATE 1

typedef enum
{
    NOT_JOINED,
    JOINING,
    JOINED

} JoinStateEnum;

class ConnectionState
{
    PersistedValue<JoinStateEnum, PWM_ID_NODE_STATE> state;

public:
    ConnectionState();

    void init();
};

#endif // CONNECTIONSTATE_H

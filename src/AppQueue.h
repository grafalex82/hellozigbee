#ifndef APP_QUEUE_H
#define APP_QUEUE_H

#include "Queue.h"

typedef enum
{
    BUTTON_VERY_LONG_PRESS
} ApplicationEventType;

struct ApplicationEvent
{
    ApplicationEventType eventType;
    uint8 buttonId;
};

extern Queue<ApplicationEvent, 5> appEventQueue;

const char * getApplicationEventName(ApplicationEventType evtType);

#endif //APP_QUEUE_H

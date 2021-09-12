#ifndef APP_QUEUE_H
#define APP_QUEUE_H

#include "Queue.h"

typedef enum
{
    BUTTON_PRESS,
    BUTTON_RELEASE,
    VERY_LONG_PRESS
} ApplicationEventType;

struct ApplicationEvent
{
    ApplicationEventType eventType;
    uint8 buttonId;
    uint32 timeStamp;
};


extern Queue<ApplicationEvent, 5> appEventQueue;

#endif //APP_QUEUE_H

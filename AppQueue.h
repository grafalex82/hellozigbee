#ifndef APP_QUEUE_H
#define APP_QUEUE_H

#include "Queue.h"

typedef enum
{
        BUTTON_SHORT_PRESS,
        BUTTON_LONG_PRESS
} ApplicationEvent;

extern Queue<ApplicationEvent, 5> appEventQueue;

#endif //APP_QUEUE_H
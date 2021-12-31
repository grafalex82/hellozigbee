#include "AppQueue.h"

Queue<ApplicationEvent, 5> appEventQueue;

const char * getApplicationEventName(ApplicationEventType evtType)
{
    switch(evtType)
    {
    case BUTTON_VERY_LONG_PRESS: return "BUTTON_VERY_LONG_PRESS";
    }

    // Should never happen
    return "";
}

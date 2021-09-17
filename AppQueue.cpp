#include "AppQueue.h"

Queue<ApplicationEvent, 5> appEventQueue;

const char * getApplicationEventName(ApplicationEventType evtType)
{
    switch(evtType)
    {
    case SWITCH_TRIGGER: return "SWITCH_TRIGGER";
    case SWITCH_ON: return "SWITCH_ON";
    case SWITCH_OFF: return "SWITCH_OFF";
    case BUTTON_PRESSED: return "BUTTON_PRESSED";
    case BUTTON_RELEASED: return "BUTTON_RELEASED";
    case BUTTON_ACTION_SINGLE: return "BUTTON_ACTION_SINGLE";
    case BUTTON_ACTION_DOUBLE: return "BUTTON_ACTION_DOUBLE";
    case BUTTON_ACTION_TRIPPLE: return "BUTTON_ACTION_TRIPPLE";
    case BUTTON_VERY_LONG_PRESS: return "BUTTON_VERY_LONG_PRESS";
    }

    // Should never happen
    return "";
}

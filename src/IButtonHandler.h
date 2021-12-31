#ifndef IBUTTONHANDLER_H
#define IBUTTONHANDLER_H

#include <jendefs.h>

static const uint32 ButtonPollCycle = 10;

class IButtonHandler
{
public:
	// Executed by ButtonsTask every ButtonPollCycle ms for every handler
	virtual void handleButtonState(bool pressed) = 0;
	virtual void resetButtonStateMachine() = 0;
};


#endif //IBUTTONHANDLER_H

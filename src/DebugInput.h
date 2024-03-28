#ifndef DEBUG_INPUT_H
#define DEBUG_INPUT_H

class DebugInput
{
    static const int BUF_SIZE = 32;
    char buf[BUF_SIZE];
    char * ptr;
    bool hasData;

public:
    DebugInput();

    void handleDebugInput();
    bool hasCompletedLine() const;

    bool matchCommand(const char * command) const;
    void reset();
};

void APP_vHandleDebugInput(DebugInput & debugInput);

#endif // DEBUG_INPUT_H

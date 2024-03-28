#ifndef DEBUG_INPUT_H
#define DEBUG_INPUT_H

class DebugInput
{
    static const int BUF_SIZE = 32;
    char buf[BUF_SIZE];
    char * ptr;
    bool hasData;

private:
    DebugInput();

    void readUart();
    bool hasCompletedLine() const;

    bool matchCommand(const char * command) const;
    void reset();

public:
    static DebugInput & getInstance();
    void handleInput();
};

#endif // DEBUG_INPUT_H

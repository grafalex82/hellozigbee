#ifndef DEFERREDEXECUTOR_H
#define DEFERREDEXECUTOR_H

#include "Queue.h"

#define DEFERRED_EXECUTOR_QUEUE_SIZE 20
typedef void (*deferredCallback)(uint8);

class DeferredExecutor
{
    Queue<uint32, DEFERRED_EXECUTOR_QUEUE_SIZE> delayQueue;
    Queue<deferredCallback, DEFERRED_EXECUTOR_QUEUE_SIZE> callbacksQueue;
    Queue<uint8, DEFERRED_EXECUTOR_QUEUE_SIZE> paramsQueue;

    uint8 timerHandle;

public:
    DeferredExecutor();

    void init();
    void runLater(uint32 delay, deferredCallback cb, uint8 param);

private:
    static void timerCallback(void *timerParam);
    void scheduleNextCall();
};

#endif // DEFERREDEXECUTOR_H

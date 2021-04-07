#ifndef QUEUE_H
#define QUEUE_H

extern "C"
{
    #include "ZQueue.h"
}

template<class T, uint32 size>
class Queue
{
    tszQueue queueHandle;
    T queueStorage[size];

public:
    Queue()
    {
        // JN5169 CRT does not really call constrictors for global object
        DBG_vPrintf(TRUE, "In a queue constructor...\n");
    }

    void init()
    {
        ZQ_vQueueCreate(&queueHandle, size, sizeof(T), (uint8*)queueStorage);
    }

    bool receive(T * val)
    {
        return ZQ_bQueueReceive(&queueHandle, (uint8*)val) != 0;
    }

    void send(const T & val)
    {
        ZQ_bQueueSend(&queueHandle, (uint8*)&val);
    }
};

#endif // QUEUE_H

#ifndef QUEUE_H
#define QUEUE_H

extern "C"
{
    #include "ZQueue.h"
    #include "dbg.h"
}

template<tszQueue * handle>
struct QueueHandleExtStorage
{
    tszQueue * getHandle()
    {
        return handle;
    }
};

struct QueueHandleIntStorage
{
    tszQueue handle;

    tszQueue * getHandle()
    {
        return &handle;
    }
};


template<class T, uint32 size, class H>
class QueueBase : public H
{
    T queueStorage[size];

public:
    QueueBase()
    {
        // JN5169 CRT does not really call constrictors for global object
        DBG_vPrintf(TRUE, "In a queue constructor...\n");
    }

    void init()
    {
        ZQ_vQueueCreate(H::getHandle(), size, sizeof(T), (uint8*)queueStorage);
    }

    bool receive(T * val)
    {
        return ZQ_bQueueReceive(H::getHandle(), (uint8*)val) != 0;
    }

    void send(const T & val)
    {
        ZQ_bQueueSend(H::getHandle(), (uint8*)&val);
    }
};

template<class T, uint32 size>
class Queue : public QueueBase<T, size, QueueHandleIntStorage >
{};

template<class T, uint32 size, tszQueue * handle>
class QueueExt : public QueueBase<T, size, QueueHandleExtStorage<handle> >
{};



#endif // QUEUE_H

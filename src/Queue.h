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

extern const tszQueue dummyQueue;

template<class T, uint32 size, const tszQueue * handle = &dummyQueue>
class Queue : public QueueBase<T, size, QueueHandleExtStorage<handle> >
{};

template<class T, uint32 size>
class Queue<T, size> : public QueueBase<T, size, QueueHandleIntStorage >
{};




#endif // QUEUE_H

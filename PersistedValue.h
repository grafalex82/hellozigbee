#ifndef PERSISTED_VALUE
#define PERSISTED_VALUE

extern "C"
{
#include "PDM.h"
#include "dbg.h"
}

template<class T, uint8 id>
class PersistedValue
{
    T value;

public:
    void init(const T & initValue)
    {
        uint16 readBytes;
        PDM_teStatus status = PDM_eReadDataFromRecord(id, &value, sizeof(T), &readBytes);
        if(status != PDM_E_STATUS_OK)
            setValue(initValue);

        DBG_vPrintf(TRUE, "PersistedValue::init(). Status %d, value %d\n", status, value);
    }

    T getValue()
    {
        return value;
    }

    operator T()
    {
        return value;
    }

    PersistedValue<T, id> & operator =(const T & newValue)
    {
        setValue(newValue);
        return *this;
    }

    void setValue(const T & newValue)
    {
        value = newValue;
        PDM_teStatus status = PDM_eSaveRecordData(id, &value, sizeof(T));
        DBG_vPrintf(TRUE, "PersistedValue::setValue() Status %d, value %d\n", status, value);
    }
};


#endif //PERSISTED_VALUE

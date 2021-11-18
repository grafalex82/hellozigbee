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
        {
            DBG_vPrintf(TRUE, "PersistedValue::init(): no corresponding flash record found. Intializing with default value.\n");
            setValue(initValue);
        }

        if(sizeof(T) <= 4)
            DBG_vPrintf(TRUE, "PersistedValue::init(): Status %d, value %d\n", status, value);
        else
            DBG_vPrintf(TRUE, "PersistedValue::init(): Status %d\n", status);
    }

    void init(void(*initFunc)(T*))
    {
        uint16 readBytes;
        PDM_teStatus status = PDM_eReadDataFromRecord(id, &value, sizeof(T), &readBytes);
        if(status != PDM_E_STATUS_OK)
        {
            DBG_vPrintf(TRUE, "PersistedValue::init(): no corresponding flash record found. Calling initialization function\n");
            initFunc(&value);
        }

        if(sizeof(T) <= 4)
            DBG_vPrintf(TRUE, "PersistedValue::init(): Status %d, value %d\n", status, value);
        else
            DBG_vPrintf(TRUE, "PersistedValue::init(): Status %d\n", status);
    }

    T getValue() const
    {
        return value;
    }

    operator T() const
    {
        return value;
    }

    T* operator&() // Emulate a 'pointer to value' behavior. setValue() will not be called.
    {
        return &value;
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

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
    const char * name;

public:
    void init(const T & initValue, const char * varname = "")
    {
        name = varname;

        uint16 readBytes;
        PDM_teStatus status = PDM_eReadDataFromRecord(id, &value, sizeof(T), &readBytes);
        if(status != PDM_E_STATUS_OK)
        {
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: no corresponding flash record found. Intializing with default value.\n", name);
            setValue(initValue);
        }

        if(sizeof(T) <= 4)
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: size %d, Status %d, value %d\n", name, sizeof(T), status, value);
        else
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: size %d, Status %d\n", name, sizeof(T), status);
    }

    void init(void(*initFunc)(T*), const char * varname)
    {
        name = varname;

        uint16 readBytes;
        PDM_teStatus status = PDM_eReadDataFromRecord(id, &value, sizeof(T), &readBytes);
        if(status != PDM_E_STATUS_OK)
        {
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: no corresponding flash record found. Calling initialization function\n", name);
            initFunc(&value);
        }

        if(sizeof(T) <= 4)
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: size %d, Status %d, value %d\n", name, sizeof(T), status, value);
        else
            DBG_vPrintf(TRUE, "PersistedValue::init(): %s: size %d, Status %d\n", name, sizeof(T), status);
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
        if(sizeof(T) <= 4)
            DBG_vPrintf(TRUE, "PersistedValue::setValue(): %s: Status %d, value %d\n", name, status, value);
        else
            DBG_vPrintf(TRUE, "PersistedValue::setValue(): %s: Status %d\n", name, status);
    }
};


#endif //PERSISTED_VALUE

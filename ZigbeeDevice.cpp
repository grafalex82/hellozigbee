extern "C"
{
    #include "jendefs.h"

    // Local configuration and generated files
    #include "zps_gen.h"

    // ZigBee includes
    #include "zps_apl.h"
    #include "zps_apl_af.h"
    #include "bdb_api.h"
    #include "dbg.h"

    // work around of a bug in appZpsBeaconHandler.h that does not have a closing } for its extern "C" statement
    }

}

#include "ZigbeeDevice.h"
#include "DumpFunctions.h"
#include "Queue.h"

ZigbeeDevice::ZigbeeDevice()
{

    // Restore network connection state
    connectionState.init(NOT_JOINED);
}

ZigbeeDevice * ZigbeeDevice::getInstance()
{
    static ZigbeeDevice instance;
    return &instance;
}

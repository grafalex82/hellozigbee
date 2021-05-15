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

extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_TimeEvents;
extern PUBLIC tszQueue zps_msgMcpsDcfm;

QueueExt<MAC_tsMlmeVsDcfmInd, 10, &zps_msgMlmeDcfmInd> msgMlmeDcfmIndQueue;
QueueExt<MAC_tsMcpsVsDcfmInd, 24, &zps_msgMcpsDcfmInd> msgMcpsDcfmIndQueue;
QueueExt<MAC_tsMcpsVsCfmData, 5, &zps_msgMcpsDcfm> msgMcpsDcfmQueue;
QueueExt<zps_tsTimeEvent, 8, &zps_TimeEvents> timeEventQueue;

ZigbeeDevice::ZigbeeDevice()
{
    // Initialize Zigbee stack queues
    DBG_vPrintf(TRUE, "vAppMain(): init software queues...\n");
    msgMlmeDcfmIndQueue.init();
    msgMcpsDcfmIndQueue.init();
    msgMcpsDcfmQueue.init();
    timeEventQueue.init();

    // Restore network connection state
    connectionState.init(NOT_JOINED);
}

ZigbeeDevice * ZigbeeDevice::getInstance()
{
    static ZigbeeDevice instance;
    return &instance;
}

PUBLIC void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent);
PUBLIC void vHandleNetworkJoinAndRejoin();
PUBLIC void vHandleRejoinFailure();

void ZigbeeDevice::handleBdbEvent(BDB_tsBdbEvent *psBdbEvent)
{
    DBG_vPrintf(TRUE, "ZigbeeDevice::handleBdbEvent\n");
    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_ZPSAF:
            vAppHandleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: BDB Init Successful\n");
            break;

        case BDB_EVENT_REJOIN_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: Network Join Successful\n");
            vHandleNetworkJoinAndRejoin();
            break;

        case BDB_EVENT_REJOIN_FAILURE:
            DBG_vPrintf(TRUE, "BDB event callback: Failed to rejoin\n");
            vHandleRejoinFailure();
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: Network steering success\n");
            vHandleNetworkJoinAndRejoin();
            break;

        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRUE, "BDB event callback: No good network to join\n");
            vHandleRejoinFailure();
            break;

        case BDB_EVENT_FAILURE_RECOVERY_FOR_REJOIN:
            DBG_vPrintf(TRUE, "BDB event callback: Failure recovery for rejoin\n");
            break;

        default:
            DBG_vPrintf(1, "BDB event callback: evt %d\n", psBdbEvent->eEventType);
            break;
    }
}

PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent * event)
{
    DBG_vPrintf(TRUE, "APP_vBdbCallback\n");
    ZigbeeDevice::getInstance()->handleBdbEvent(event);
}

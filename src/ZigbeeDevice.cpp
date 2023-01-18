#include "ZigbeeDevice.h"
#include "DumpFunctions.h"
#include "Queue.h"

extern "C"
{
    #include "jendefs.h"

    // Local configuration and generated files
    #include "zps_gen.h"
    #include "pdum_gen.h"

    // ZigBee includes
    #include "zps_apl.h"
    #include "zps_apl_af.h"
    #include "bdb_api.h"
    #include "dbg.h"
    #include "OnOff.h"
}


extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_TimeEvents;
extern PUBLIC tszQueue zps_msgMcpsDcfm;

Queue<MAC_tsMlmeVsDcfmInd, 10, &zps_msgMlmeDcfmInd> msgMlmeDcfmIndQueue;
Queue<MAC_tsMcpsVsDcfmInd, 24, &zps_msgMcpsDcfmInd> msgMcpsDcfmIndQueue;
Queue<MAC_tsMcpsVsCfmData, 5, &zps_msgMcpsDcfm> msgMcpsDcfmQueue;
Queue<zps_tsTimeEvent, 8, &zps_TimeEvents> timeEventQueue;

ZigbeeDevice::ZigbeeDevice()
{
    // Initialize Zigbee stack queues
    DBG_vPrintf(TRUE, "ZigbeeDevice(): init zigbee queues...\n");
    msgMlmeDcfmIndQueue.init();
    msgMcpsDcfmIndQueue.init();
    msgMcpsDcfmQueue.init();
    timeEventQueue.init();

    // Restore network connection state
    connectionState.init(NOT_JOINED);

    // Initialise Application Framework stack
    DBG_vPrintf(TRUE, "ZigbeeDevice(): init Application Framework (AF)... ");
    ZPS_teStatus status = ZPS_eAplAfInit();
    DBG_vPrintf(TRUE, "ZPS_eAplAfInit() status %d\n", status);

    // Initialize Base Class Behavior
    DBG_vPrintf(TRUE, "ZigbeeDevice(): initialize base device behavior...\n");
    bdbEventQueue.init();
    BDB_tsInitArgs sInitArgs;
    sInitArgs.hBdbEventsMsgQ = bdbEventQueue.getHandle();
    BDB_vInit(&sInitArgs);

    polling = false;
    rejoinFailures = 0;
}

ZigbeeDevice * ZigbeeDevice::getInstance()
{
    static ZigbeeDevice instance;
    return &instance;
}

void ZigbeeDevice::joinNetwork()
{
    DBG_vPrintf(TRUE, "== Joining the network\n");
    connectionState = JOINING;

    // Clear ZigBee stack internals
    sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
    sBDB.sAttrib.u8bdbCommissioningMode = BDB_COMMISSIONING_MODE_NWK_STEERING;
    ZPS_eAplAibSetApsUseExtendedPanId(0);
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();

    // Connect to a network
    ZPS_teStatus status = BDB_eNsStartNwkSteering();
    DBG_vPrintf(TRUE, "  BDB_eNsStartNwkSteering=%d\n", status);
}

void ZigbeeDevice::rejoinNetwork()
{
    DBG_vPrintf(TRUE, "== Rejoining the network\n");

    sBDB.sAttrib.bbdbNodeIsOnANetwork = (connectionState == JOINED ? TRUE : FALSE);
    sBDB.sAttrib.u8bdbCommissioningMode = BDB_COMMISSIONING_MODE_NWK_STEERING;

    DBG_vPrintf(TRUE, "ZigbeeDevice(): Starting base device behavior... bNodeIsOnANetwork=%d\n", sBDB.sAttrib.bbdbNodeIsOnANetwork);
    ZPS_vSaveAllZpsRecords();
    BDB_vStart();
}

void ZigbeeDevice::leaveNetwork()
{
    DBG_vPrintf(TRUE, "== Leaving the network\n");
    sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
    connectionState = NOT_JOINED;
    rejoinFailures = 0;

    if (ZPS_E_SUCCESS !=  ZPS_eAplZdoLeaveNetwork(0, FALSE, FALSE))
    {
        // Leave failed, probably lost parent, so just reset everything
        DBG_vPrintf(TRUE, "== Failed to properly leave the network. Force leaving the network\n");
        handleLeaveNetwork();
     }
}

void ZigbeeDevice::joinOrLeaveNetwork()
{
    if(connectionState == JOINED)
        leaveNetwork();
    else
        joinNetwork();
}

bool ZigbeeDevice::isJoined()
{
    return connectionState == JOINED;
}

void ZigbeeDevice::handleNetworkJoinAndRejoin()
{
    DBG_vPrintf(TRUE, "== Device now is on the network\n");
    connectionState = JOINED;

    // Store the extended network ID for future use
    ZPS_eAplAibSetApsUseExtendedPanId(ZPS_u64NwkNibGetEpid(ZPS_pvAplZdoGetNwkHandle()));

    ZPS_vSaveAllZpsRecords();

    // Get ready for network communication
    if(ZPS_eAplZdoGetDeviceType() == ZPS_ZDO_DEVICE_ENDDEVICE)
        pollTask.startPoll(2000);
    rejoinFailures = 0;
}

void ZigbeeDevice::handleLeaveNetwork()
{
    DBG_vPrintf(TRUE, "== The device has left the network\n");

    connectionState = NOT_JOINED;

    pollTask.stopPoll();

    // Clear ZigBee stack internals
    ZPS_eAplAibSetApsUseExtendedPanId (0);
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();
}

void ZigbeeDevice::handleRejoinFailure()
{
    DBG_vPrintf(TRUE, "== Failed to (re)join the network\n");
    polling = false;

    if(connectionState == JOINED && ++rejoinFailures < 5)
    {
        DBG_vPrintf(TRUE, "  Rejoin counter %d\n", rejoinFailures);

        // Schedule sleep for a minute
        cyclesTillNextRejoin = 4; // 4 * 15s = 1 minute
    }
    else
        handleLeaveNetwork();
}

void ZigbeeDevice::handlePollResponse(ZPS_tsAfPollConfEvent* pEvent)
{
    switch (pEvent->u8Status)
    {
        case MAC_ENUM_SUCCESS:
        case MAC_ENUM_NO_ACK:
            pollParent();
            break;

        case MAC_ENUM_NO_DATA:
            polling = false;
        default:
            break;
    }
}

void ZigbeeDevice::handleZdoDataIndication(ZPS_tsAfEvent * pEvent)
{
    ZPS_tsAfZdpEvent zdpEvent;

    switch(pEvent->uEvent.sApsDataIndEvent.u16ClusterId)
    {
        case ZPS_ZDP_ACTIVE_EP_RSP_CLUSTER_ID:
        {
            bool res = zps_bAplZdpUnpackActiveEpResponse(pEvent, &zdpEvent);
            DBG_vPrintf(TRUE, "Unpacking Active Endpoint Response: Status: %02x res:%02x\n", zdpEvent.uZdpData.sActiveEpRsp.u8Status, res);
            for(uint8 i=0; i<zdpEvent.uZdpData.sActiveEpRsp.u8ActiveEpCount; i++)
            {
                uint8 ep = zdpEvent.uLists.au8Data[i];
                DBG_vPrintf(TRUE, "Scheduling simple descriptor request for EP %d\n", ep);

                //deferredExecutor.runLater(1000, vSendSimpleDescriptorReq, ep);
            }
        }
    }
}

void ZigbeeDevice::handleZdoBindUnbindEvent(ZPS_tsAfZdoBindEvent * pEvent, bool bind)
{
    if(!bind)
        return;

    // Address of interest
    ZPS_tsAplZdpNwkAddrReq req = {pEvent->uDstAddr.u64Addr, 0, 0};

    // Target addr (Broadcast)
    ZPS_tuAddress uDstAddr;
    uDstAddr.u16Addr = 0xFFFF;   // Broadcast

    // Perform the request
    uint8 u8SeqNumber;
    PDUM_thAPduInstance hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);
    ZPS_teStatus status = ZPS_eAplZdpNwkAddrRequest(hAPduInst,
                                                    uDstAddr,     // Broadcast addr
                                                    FALSE,
                                                    &u8SeqNumber,
                                                    &req);
    DBG_vPrintf(TRUE, "ZigbeeDevice::handleZdoBindUnbindEvent(): looking for network addr for %016llx. Status=%02x\n", pEvent->uDstAddr.u64Addr, status);
}

void ZigbeeDevice::handleZclEvents(ZPS_tsAfEvent* psStackEvent)
{
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = psStackEvent;
    sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
    vZCL_EventHandler(&sCallBackEvent);
}

void ZigbeeDevice::handleZdoEvents(ZPS_tsAfEvent* psStackEvent)
{
    if(connectionState != JOINED)
    {
        DBG_vPrintf(TRUE, "Handle ZDO event: Not joined yet. Discarding event %d\n", psStackEvent->eType);
        return;
    }

    switch(psStackEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            handleZdoDataIndication(psStackEvent);
            break;

        case ZPS_EVENT_NWK_LEAVE_INDICATION:
            if(psStackEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0)
                handleLeaveNetwork();
            break;

        case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            handleLeaveNetwork();
            break;

        case ZPS_EVENT_ZDO_BIND:
            handleZdoBindUnbindEvent(&psStackEvent->uEvent.sZdoBindEvent, true);
            break;

        case ZPS_EVENT_ZDO_UNBIND:
            handleZdoBindUnbindEvent(&psStackEvent->uEvent.sZdoBindEvent, false);
            break;

        case ZPS_EVENT_NWK_POLL_CONFIRM:
            handlePollResponse(&psStackEvent->uEvent.sNwkPollConfirmEvent);
            break;

        default:
            //DBG_vPrintf(TRUE, "Handle ZDO event: event type %d\n", psStackEvent->eType);
            break;
    }
}

void ZigbeeDevice::handleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    // Dump the event for debug purposes
    vDumpAfEvent(&psZpsAfEvent->sStackEvent);

    if(psZpsAfEvent->u8EndPoint == HELLOENDDEVICE_ZDO_ENDPOINT)
    {
        // events for ep 0
        handleZdoEvents(&psZpsAfEvent->sStackEvent);
    }
    else if(psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        handleZclEvents(&psZpsAfEvent->sStackEvent);
    }
    else if (psZpsAfEvent->sStackEvent.eType != ZPS_EVENT_APS_DATA_CONFIRM &&
             psZpsAfEvent->sStackEvent.eType != ZPS_EVENT_APS_DATA_ACK)
    {
        DBG_vPrintf(TRUE, "AF event callback: endpoint %d, event %d\n", psZpsAfEvent->u8EndPoint, psZpsAfEvent->sStackEvent.eType);
    }

    // Ensure Freeing of APDUs
    if(psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
}

void ZigbeeDevice::handleBdbEvent(BDB_tsBdbEvent *psBdbEvent)
{
    switch(psBdbEvent->eEventType)
    {
        case BDB_EVENT_ZPSAF:
            handleAfEvent(&psBdbEvent->uEventData.sZpsAfEvent);
            break;

        case BDB_EVENT_INIT_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: BDB Init Successful\n");
            break;

        case BDB_EVENT_REJOIN_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: Network Join Successful\n");
            handleNetworkJoinAndRejoin();
            break;

        case BDB_EVENT_REJOIN_FAILURE:
            DBG_vPrintf(TRUE, "BDB event callback: Failed to rejoin\n");
            handleRejoinFailure();
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: Network steering success\n");
            handleNetworkJoinAndRejoin();
            break;

        case BDB_EVENT_NO_NETWORK:
            DBG_vPrintf(TRUE, "BDB event callback: No good network to join\n");
            handleRejoinFailure();
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
    ZigbeeDevice::getInstance()->handleBdbEvent(event);
}

void ZigbeeDevice::pollParent()
{
    // Sleeping devices have to poll their parents, while routers always keep the receiver on
    // Note: this condition should check sleeping/non-sleeping device criteria, rather than device type.
    //       Perhaps even end device may be a non-sleeping device, and therefore not require polling.
    if(ZPS_eAplZdoGetDeviceType() != ZPS_ZDO_DEVICE_ENDDEVICE)
        return;

    polling = true;
    DBG_vPrintf(TRUE, "ZigbeeDevice: Polling parent for zigbee messages\n");
    ZPS_eAplZdoPoll();
}

bool ZigbeeDevice::canSleep() const
{
    // It is assumed that only end devices can sleep.
    // Note: this condition should check sleeping/non-sleeping device criteria, rather than device type.
    //       Perhaps even end device may be a non-sleeping device, and therefore not require polling.
    if(ZPS_eAplZdoGetDeviceType() != ZPS_ZDO_DEVICE_ENDDEVICE)
        return false;

    // End device may sleep if they are not polling the parent right now
    return !polling;
}

bool ZigbeeDevice::needsRejoin() const
{
    // Non-zero rejoin failure counter reflects that we have received spontaneous
    // Rejoin failure message while the node was in JOINED state
    return rejoinFailures > 0 && connectionState == JOINED;
}

void ZigbeeDevice::handleWakeUp()
{
    if(connectionState != JOINED)
        return;

    if(needsRejoin())
    {
        // Device that is basically connected, but currently needs a rejoin will have to
        // sleep a few cycles between rejoin attempts
        if(cyclesTillNextRejoin-- > 0)
        {
            DBG_vPrintf(TRUE, "ZigbeeDevice: Rejoining in %d cycles\n", cyclesTillNextRejoin);
            return;
        }

        rejoinNetwork();
    }
    else
        // Connected device will just poll its parent on wake up
        pollParent();
}

extern "C"
{
    #include "AppHardwareApi.h"
    #include "dbg.h"
    #include "dbg_uart.h"
    #include "ZTimer.h"
    #include "ZQueue.h"
    #include "portmacro.h"
    #include "pwrm.h"
    #include "PDM.h"

    // Local configuration and generated files
    #include "pdum_gen.h"
    #include "zps_gen.h"

    // ZigBee includes
    #include "zcl.h"
    #include "zps_apl.h"
    #include "zps_apl_af.h"
    #include "bdb_api.h"

    // work around of a bug in appZpsBeaconHandler.h that does not have a closing } for its extern "C" statement
    }

    // Zigbee cluster includes
    #include "on_off_light.h"
    #include "OnOff.h"
}

#include "Queue.h"
#include "Timer.h"
#include "DeferredExecutor.h"
#include "BlinkTask.h"
#include "ButtonsTask.h"
#include "PollTask.h"
#include "AppQueue.h"
#include "DumpFunctions.h"
#include "SwitchEndpoint.h"
#include "EndpointManager.h"
#include "ZigbeeDevice.h"

DeferredExecutor deferredExecutor;

// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}


#define PDM_ID_BLINK_MODE   	    0x2



ZTIMER_tsTimer timers[4 + BDB_ZTIMER_STORAGE];




extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_TimeEvents;
extern PUBLIC tszQueue zps_msgMcpsDcfm;

QueueExt<MAC_tsMlmeVsDcfmInd, 10, &zps_msgMlmeDcfmInd> msgMlmeDcfmIndQueue;
QueueExt<MAC_tsMcpsVsDcfmInd, 24, &zps_msgMcpsDcfmInd> msgMcpsDcfmIndQueue;
QueueExt<MAC_tsMcpsVsCfmData, 5, &zps_msgMcpsDcfm> msgMcpsDcfmQueue;
QueueExt<zps_tsTimeEvent, 8, &zps_TimeEvents> timeEventQueue;


struct Context
{
    SwitchEndpoint switch1;
};


extern "C" void __cxa_pure_virtual(void) __attribute__((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__((__noreturn__));

void __cxa_pure_virtual(void)
{
  // We might want to write some diagnostics to uart in this case
  //std::terminate();
  while (1)
    ;
}


extern "C" PUBLIC void vISR_SystemController(void)
{
}

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {

        case E_ZCL_CBET_UNHANDLED_EVENT:
            DBG_vPrintf(TRUE, "ZCL General Callback: Unhandled Event\n");
            break;

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL General Callback: Read attributes response\n");
            break;

        case E_ZCL_CBET_READ_REQUEST:
            DBG_vPrintf(TRUE, "ZCL General Callback: Read request\n");
            break;

        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL General Callback: Default response\n");
            break;

        case E_ZCL_CBET_ERROR:
            DBG_vPrintf(TRUE, "ZCL General Callback: Error\n");
            break;

        case E_ZCL_CBET_TIMER:
            break;

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRUE, "ZCL General Callback: ZigBee\n");
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRUE, "ZCL General Callback: Custom\n");
            break;

        default:
            DBG_vPrintf(TRUE, "ZCL General Callback: Invalid event type (%d) in APP_ZCL_cbGeneralCallback\n", psEvent->eEventType);
            break;
    }
}

void vHandlePollResponse(ZPS_tsAfPollConfEvent* pEvent)
{
    switch (pEvent->u8Status)
    {
        case MAC_ENUM_SUCCESS:
        case MAC_ENUM_NO_ACK:
            ZPS_eAplZdoPoll();
            break;

        case MAC_ENUM_NO_DATA:
        default:
            break;
    }

}

PRIVATE void vJoinNetwork()
{
    DBG_vPrintf(TRUE, "== Joining the network\n");
    ZigbeeDevice::getInstance()->setState(JOINING);

    // Clear ZigBee stack internals
    ZPS_eAplAibSetApsUseExtendedPanId (0);
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();

    // Connect to a network
    BDB_eNsStartNwkSteering();
}

PUBLIC void vHandleNetworkJoinAndRejoin()
{
    DBG_vPrintf(TRUE, "== Device now is on the network\n");
    ZigbeeDevice::getInstance()->setState(JOINED);
    ZPS_vSaveAllZpsRecords();

    PollTask::getInstance().startPoll(2000);
}

PRIVATE void vHandleLeaveNetwork()
{
    DBG_vPrintf(TRUE, "== The device has left the network\n");

    ZigbeeDevice::getInstance()->setState(NOT_JOINED);

    PollTask::getInstance().stopPoll();

    // Clear ZigBee stack internals
    ZPS_eAplAibSetApsUseExtendedPanId (0);
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    ZPS_vSaveAllZpsRecords();
}

PUBLIC void vHandleRejoinFailure()
{
    DBG_vPrintf(TRUE, "== Failed to (re)join the network\n");

    vHandleLeaveNetwork();
}


PRIVATE void vLeaveNetwork()
{
    DBG_vPrintf(TRUE, "== Leaving the network\n");
    sBDB.sAttrib.bbdbNodeIsOnANetwork = FALSE;
    ZigbeeDevice::getInstance()->setState(NOT_JOINED);

    if (ZPS_E_SUCCESS !=  ZPS_eAplZdoLeaveNetwork(0, FALSE, FALSE))
    {
        // Leave failed, probably lost parent, so just reset everything
        DBG_vPrintf(TRUE, "== Failed to properly leave the network. Force leaving the network\n");
        vHandleLeaveNetwork();
     }
}

void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRUE,"ERROR: Extended status %x\n", eExtendedStatus);
}

PRIVATE void vGetCoordinatorEndpoints(uint8)
{
    PDUM_thAPduInstance hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);

    // Destination address - 0x0000 (Coordinator)
    ZPS_tuAddress uDstAddr;
    uDstAddr.u16Addr = 0;

    // Active Endpoints request
    ZPS_tsAplZdpActiveEpReq sNodeDescReq;
    sNodeDescReq.u16NwkAddrOfInterest = uDstAddr.u16Addr;

    // Send the request
    uint8 u8SeqNumber;
    ZPS_teStatus status = ZPS_eAplZdpActiveEpRequest(hAPduInst,
                                                     uDstAddr,
                                                     FALSE,       // bExtAddr
                                                     &u8SeqNumber,
                                                     &sNodeDescReq);

    DBG_vPrintf(TRUE, "Sent Active endpoints request to coordinator %d\n", status);
}

PRIVATE void vSendSimpleDescriptorReq(uint8 ep)
{
    PDUM_thAPduInstance hAPduInst = PDUM_hAPduAllocateAPduInstance(apduZDP);

    // Destination address - 0x0000 (Coordinator)
    ZPS_tuAddress uDstAddr;
    uDstAddr.u16Addr = 0;

    // Simple Descriptor request
    ZPS_tsAplZdpSimpleDescReq sSimpleDescReq;
    sSimpleDescReq.u16NwkAddrOfInterest = uDstAddr.u16Addr;
    sSimpleDescReq.u8EndPoint = ep;

    // Send the request
    uint8 u8SeqNumber;
    ZPS_teStatus status = ZPS_eAplZdpSimpleDescRequest(hAPduInst,
                                                     uDstAddr,
                                                     FALSE,       // bExtAddr
                                                     &u8SeqNumber,
                                                     &sSimpleDescReq);

    DBG_vPrintf(TRUE, "Sent Simple Descriptor request to coordinator for EP %d (status %d)\n", ep, status);
}


PRIVATE void vHandleZdoDataIndication(ZPS_tsAfEvent * pEvent)
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

                deferredExecutor.runLater(1000, vSendSimpleDescriptorReq, ep);
            }
        }
    }
}

PRIVATE void vHandleZdoBindEvent(ZPS_tsAfZdoBindEvent * pEvent)
{
    ZPS_teStatus status = ZPS_eAplZdoBind(GENERAL_CLUSTER_ID_ONOFF,
                                          pEvent->u8SrcEp,
                                          0x2C9C,
                                          pEvent->uDstAddr.u64Addr,
                                          pEvent->u8DstEp);
    DBG_vPrintf(TRUE, "Binding SrcEP=%d to DstEP=%d Status=%d\n", pEvent->u8SrcEp, pEvent->u8DstEp, status);
}

PRIVATE void vHandleZdoUnbindEvent(ZPS_tsAfZdoUnbindEvent * pEvent)
{

}


PRIVATE void vAppHandleZdoEvents(ZPS_tsAfEvent* psStackEvent)
{
    if(ZigbeeDevice::getInstance()->getState() != JOINED)
    {
        DBG_vPrintf(TRUE, "Handle ZDO event: Not joined yet. Discarding event %d\n", psStackEvent->eType);
        return;
    }

    switch(psStackEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            vHandleZdoDataIndication(psStackEvent);
            break;

        case ZPS_EVENT_NWK_LEAVE_INDICATION:
            if(psStackEvent->uEvent.sNwkLeaveIndicationEvent.u64ExtAddr == 0)
                vHandleLeaveNetwork();
            break;

        case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            vHandleLeaveNetwork();
            break;

        case ZPS_EVENT_ZDO_BIND:
            vHandleZdoBindEvent(&psStackEvent->uEvent.sZdoBindEvent);
            break;

        case ZPS_EVENT_ZDO_UNBIND:
            vHandleZdoUnbindEvent(&psStackEvent->uEvent.sZdoBindEvent);
            break;

        case ZPS_EVENT_NWK_POLL_CONFIRM:
            vHandlePollResponse(&psStackEvent->uEvent.sNwkPollConfirmEvent);
            break;

        default:
            //DBG_vPrintf(TRUE, "Handle ZDO event: event type %d\n", psStackEvent->eType);
            break;
    }
}


PRIVATE void vAppHandleZclEvents(ZPS_tsAfEvent* psStackEvent)
{
    tsZCL_CallBackEvent sCallBackEvent;
    sCallBackEvent.pZPSevent = psStackEvent;
    sCallBackEvent.eEventType = E_ZCL_CBET_ZIGBEE_EVENT;
    vZCL_EventHandler(&sCallBackEvent);
}

PUBLIC void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    // Dump the event for debug purposes
    vDumpAfEvent(&psZpsAfEvent->sStackEvent);

    if(psZpsAfEvent->u8EndPoint == HELLOENDDEVICE_ZDO_ENDPOINT)
    {
        // events for ep 0
        vAppHandleZdoEvents(&psZpsAfEvent->sStackEvent);
    }
    else if(psZpsAfEvent->u8EndPoint == HELLOENDDEVICE_SWITCH_ENDPOINT &&
            psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        vAppHandleZclEvents(&psZpsAfEvent->sStackEvent);
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

PRIVATE void APP_vTaskSwitch(Context * context)
{
    ApplicationEvent value;
    if(appEventQueue.receive(&value))
    {
        DBG_vPrintf(TRUE, "Processing button message %d\n", value);

        if(value == BUTTON_SHORT_PRESS)
        {
            context->switch1.toggle();
        }

        if(value == BUTTON_LONG_PRESS)
        {
            if(ZigbeeDevice::getInstance()->getState() == JOINED)
                vLeaveNetwork();
            else
                vJoinNetwork();
        }
    }
}

extern "C" PUBLIC void vAppMain(void)
{
    // Initialize the hardware
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();

    // Initialize UART
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    // Initialize PDM
    DBG_vPrintf(TRUE, "vAppMain(): init PDM...\n");
    PDM_eInitialise(0);
    DBG_vPrintf(TRUE, "vAppMain(): PDM Capacity %d Occupancy %d\n",
            u8PDM_CalculateFileSystemCapacity(),
            u8PDM_GetFileSystemOccupancy() );

    // Initialize power manager and sleep mode
    DBG_vPrintf(TRUE, "vAppMain(): init PWRM...\n");
    PWRM_vInit(E_AHI_SLEEP_DEEP);

    // PDU Manager initialization
    DBG_vPrintf(TRUE, "vAppMain(): init PDUM...\n");
    PDUM_vInit();

    // Init timers
    DBG_vPrintf(TRUE, "vAppMain(): init software timers...\n");
    ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));

    // Init tasks
    DBG_vPrintf(TRUE, "vAppMain(): init tasks...\n");
    ButtonsTask buttonsTask;

    // Initialize ZigBee stack and application queues
    DBG_vPrintf(TRUE, "vAppMain(): init software queues...\n");
    msgMlmeDcfmIndQueue.init();
    msgMcpsDcfmIndQueue.init();
    msgMcpsDcfmQueue.init();
    timeEventQueue.init();
    appEventQueue.init();

    // Initialize deferred executor
    DBG_vPrintf(TRUE, "vAppMain(): Initialize deferred executor...\n");
    deferredExecutor.init();

    // Set up a status callback
    DBG_vPrintf(TRUE, "vAppMain(): init extended status callback...\n");
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    DBG_vPrintf(TRUE, "vAppMain(): init Zigbee Class Library (ZCL)...  ");
    ZPS_teStatus status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    DBG_vPrintf(TRUE, "eZCL_Initialise() status %d\n", status);

    DBG_vPrintf(TRUE, "vAppMain(): Registering endpoint objects\n");
    Context context;
    EndpointManager::getInstance()->registerEndpoint(HELLOENDDEVICE_SWITCH_ENDPOINT, &context.switch1);

    // Force creating ZigbeeDevice here
    ZigbeeDevice::getInstance();

    // Initialise Application Framework stack
    DBG_vPrintf(TRUE, "vAppMain(): init Application Framework (AF)... ");
    status = ZPS_eAplAfInit();
    DBG_vPrintf(TRUE, "ZPS_eAplAfInit() status %d\n", status);

    // Initialize Base Class Behavior
    DBG_vPrintf(TRUE, "vAppMain(): initialize base device behavior...\n");
    Queue<BDB_tsZpsAfEvent, 3> bdbEventQueue;
    bdbEventQueue.init();

    BDB_tsInitArgs sInitArgs;
    sInitArgs.hBdbEventsMsgQ = bdbEventQueue.getHandle();
    BDB_vInit(&sInitArgs);

    sBDB.sAttrib.bbdbNodeIsOnANetwork = (ZigbeeDevice::getInstance()->getState() == JOINED ? TRUE : FALSE);
    sBDB.sAttrib.u8bdbCommissioningMode = BDB_COMMISSIONING_MODE_NWK_STEERING;
    DBG_vPrintf(TRUE, "vAppMain(): Starting base device behavior... bNodeIsOnANetwork=%d\n", sBDB.sAttrib.bbdbNodeIsOnANetwork);
    ZPS_vSaveAllZpsRecords();
    BDB_vStart();


    DBG_vPrintf(TRUE, "vAppMain(): Starting the main loop\n");
    while(1)
    {
        zps_taskZPS();

        bdb_taskBDB();

        ZTIMER_vTask();

        APP_vTaskSwitch(&context);

        vAHI_WatchdogRestart();
    }
}

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRUE, "Going to sleep..\n\n");
    DBG_vUartFlush();

    ZTIMER_vSleep();

    // Disable UART (if enabled)
    vAHI_UartDisable(E_AHI_UART_0);

    // clear interrupts
    u32AHI_DioWakeStatus();
}

PWRM_CALLBACK(Wakeup)
{
    // Stabilise the oscillator
    while (bAHI_GetClkSource() == TRUE);

    // Now we are running on the XTAL, optimise the flash memory wait states
    vAHI_OptimiseWaitStates();

    // Re-initialize Debug UART
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    DBG_vPrintf(TRUE, "\nWaking..\n");
    DBG_vUartFlush();

    // Re-initialize hardware and interrupts
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();

    // Wake the timers
    ZTIMER_vWake();
}

extern "C" void vAppRegisterPWRMCallbacks(void)
{
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);	
}

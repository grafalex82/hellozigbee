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
#include "AppQueue.h"
#include "DumpFunctions.h"

DeferredExecutor deferredExecutor;

// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}


#define PDM_ID_BLINK_MODE   	    0x2

tsZLO_OnOffLightDevice sSwitch;


ZTIMER_tsTimer timers[3 + BDB_ZTIMER_STORAGE];




extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_TimeEvents;
extern PUBLIC tszQueue zps_msgMcpsDcfm;

QueueExt<MAC_tsMlmeVsDcfmInd, 10, &zps_msgMlmeDcfmInd> msgMlmeDcfmIndQueue;
QueueExt<MAC_tsMcpsVsDcfmInd, 24, &zps_msgMcpsDcfmInd> msgMcpsDcfmIndQueue;
QueueExt<MAC_tsMcpsVsCfmData, 5, &zps_msgMcpsDcfm> msgMcpsDcfmQueue;
QueueExt<zps_tsTimeEvent, 8, &zps_TimeEvents> timeEventQueue;

extern "C" PUBLIC void vISR_SystemController(void)
{
}

PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "ZCL General Callback: Processing event %d\n", psEvent->eEventType);

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

PRIVATE void vHandleCustomClusterMessage(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 u8CommandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Custom cluster message EP=%d ClusterID=%04x Cmd=%02x\n", psEvent->u8EndPoint, u16ClusterId, u8CommandId);
}

PRIVATE void vHandleClusterUpdateMessage(tsZCL_CallBackEvent *psEvent)
{
    uint16 u16ClusterId = psEvent->uMessage.sClusterCustomMessage.u16ClusterId;
    tsCLD_OnOffCallBackMessage * msg = (tsCLD_OnOffCallBackMessage *)psEvent->uMessage.sClusterCustomMessage.pvCustomData;
    uint8 u8CommandId = msg->u8CommandId;

    DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Cluster update message EP=%d ClusterID=%04x Cmd=%02x\n", psEvent->u8EndPoint, u16ClusterId, u8CommandId);
}


PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_READ_REQUEST:
            vDumpZclReadRequest(psEvent);
            break;

        case E_ZCL_CBET_UNHANDLED_EVENT:

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:

        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: DEFAULT_RESPONSE received. No action\n");
            break;

        case E_ZCL_CBET_ERROR:

        case E_ZCL_CBET_TIMER:

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: No action (event type %d)\n", psEvent->eEventType);
            break;

        case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Read Attrib Rsp %d %02x\n", psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            vHandleCustomClusterMessage(psEvent);
            break;

        case E_ZCL_CBET_CLUSTER_UPDATE:
            vHandleClusterUpdateMessage(psEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
            break;
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


PRIVATE void vHandleDiscoveryComplete(ZPS_tsAfNwkDiscoveryEvent * pEvent)
{
    // Check if there is a suitable network to join
    if(pEvent->u8SelectedNetwork == 0xff)
    {
        DBG_vPrintf(TRUE, "Network Discovery Complete: No good network to join\n");
        return;
    }

    // Join the network
    ZPS_tsNwkNetworkDescr * pNetwork = pEvent->psNwkDescriptors + pEvent->u8SelectedNetwork;
    DBG_vPrintf(TRUE, "Network Discovery Complete: Joining network %016llx\n", pNetwork->u64ExtPanId);
    ZPS_teStatus status = ZPS_eAplZdoJoinNetwork(pNetwork);
    DBG_vPrintf(TRUE, "Network Discovery Complete: ZPS_eAplZdoJoinNetwork() status %d\n", status);
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


PRIVATE void vAppHandleZdoEvents(ZPS_tsAfEvent* psStackEvent)
{
    switch(psStackEvent->eType)
    {
        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            vHandleDiscoveryComplete(&psStackEvent->uEvent.sNwkDiscoveryEvent);
            break;

        case ZPS_EVENT_APS_DATA_INDICATION:
            vHandleZdoDataIndication(psStackEvent);
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

PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    // Dump the event for debug purposes
    vDumpAfEvent(&psZpsAfEvent->sStackEvent);

    if(psZpsAfEvent->u8EndPoint == HELLOZIGBEE_ZDO_ENDPOINT)
    {
        // events for ep 0
        vAppHandleZdoEvents(&psZpsAfEvent->sStackEvent);
    }
    else if(psZpsAfEvent->u8EndPoint == HELLOZIGBEE_SWITCH_ENDPOINT &&
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


PUBLIC void APP_vBdbCallback(BDB_tsBdbEvent *psBdbEvent)
{
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

            deferredExecutor.runLater(15000, vGetCoordinatorEndpoints, 0);

            break;

        case BDB_EVENT_REJOIN_FAILURE:
            DBG_vPrintf(TRUE, "BDB event callback: Failed to rejoin\n");
            break;

        case BDB_EVENT_NWK_STEERING_SUCCESS:
            DBG_vPrintf(TRUE, "BDB event callback: Network steering success\n");
            break;

        case BDB_EVENT_FAILURE_RECOVERY_FOR_REJOIN:
            DBG_vPrintf(TRUE, "BDB event callback: Failure recovery for rejoin\n");
            break;

        default:
            DBG_vPrintf(1, "BDB event callback: evt %d\n", psBdbEvent->eEventType);
            break;
    }
}

PRIVATE void APP_vTaskSwitch()
{
    ApplicationEvent value;
    if(appEventQueue.receive(&value))
    {
        DBG_vPrintf(TRUE, "Processing button message %d\n", value);

        if(value == BUTTON_SHORT_PRESS)
        {
            // Toggle the value
            sSwitch.sOnOffServerCluster.bOnOff = sSwitch.sOnOffServerCluster.bOnOff ? FALSE : TRUE;

            // Destination address - 0x0000 (coordinator)
            tsZCL_Address addr;
            addr.uAddress.u16DestinationAddress = 0x0000;
            addr.eAddressMode = E_ZCL_AM_SHORT;

            DBG_vPrintf(TRUE, "Reporing attribute... ", value);
            PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();
            teZCL_Status status = eZCL_ReportAttribute(&addr,
                                                       GENERAL_CLUSTER_ID_ONOFF,
                                                       E_CLD_ONOFF_ATTR_ID_ONOFF,
                                                       HELLOZIGBEE_SWITCH_ENDPOINT,
                                                       1,
                                                       myPDUM_thAPduInstance);
            PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
            DBG_vPrintf(TRUE, "status: %02x\n", status);
        }

        if(value == BUTTON_LONG_PRESS)
        {
            // TODO: add network join here
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

    // Restore blink mode from EEPROM
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
    BlinkTask blinkTask(&sSwitch.sOnOffServerCluster.bOnOff);
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

    DBG_vPrintf(TRUE, "vAppMain(): register On/Off endpoint...  ");
    status = eZLO_RegisterOnOffLightEndPoint(HELLOZIGBEE_SWITCH_ENDPOINT, &APP_ZCL_cbEndpointCallback, &sSwitch);
    DBG_vPrintf(TRUE, "eApp_ZCL_RegisterEndpoint() status %d\n", status);

    //Fill Basic cluster attributes
    memcpy(sSwitch.sBasicServerCluster.au8ManufacturerName, CLD_BAS_MANUF_NAME_STR, CLD_BAS_MANUF_NAME_SIZE);
    memcpy(sSwitch.sBasicServerCluster.au8ModelIdentifier, CLD_BAS_MODEL_ID_STR, CLD_BAS_MODEL_ID_SIZE);
    memcpy(sSwitch.sBasicServerCluster.au8DateCode, CLD_BAS_DATE_STR, CLD_BAS_DATE_SIZE);
    memcpy(sSwitch.sBasicServerCluster.au8SWBuildID, CLD_BAS_SW_BUILD_STR, CLD_BAS_SW_BUILD_SIZE);
    sSwitch.sBasicServerCluster.eGenericDeviceType = E_CLD_BAS_GENERIC_DEVICE_TYPE_WALL_SWITCH;

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

    DBG_vPrintf(TRUE, "vAppMain(): Starting base device behavior...\n");
    BDB_vStart();

    // Reset Zigbee stack to a very default state
    ZPS_vDefaultStack();
    ZPS_vSetKeys();
    ZPS_eAplAibSetApsUseExtendedPanId(0);

    // Start ZigBee stack
    DBG_vPrintf(TRUE, "vAppMain(): Starting ZigBee stack... ");
    status = ZPS_eAplZdoStartStack();
    DBG_vPrintf(TRUE, "ZPS_eAplZdoStartStack() status %d\n", status);

    DBG_vPrintf(TRUE, "vAppMain(): Starting the main loop\n");
    while(1)
    {
        zps_taskZPS();

        bdb_taskBDB();

        ZTIMER_vTask();

        APP_vTaskSwitch();

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

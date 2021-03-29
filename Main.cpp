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
    #include "on_off_light_switch.h"
}

// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}


#define BOARD_LED_BIT               (17)
#define BOARD_LED_PIN               (1UL << BOARD_LED_BIT)

#define BOARD_BTN_BIT               (1)
#define BOARD_BTN_PIN               (1UL << BOARD_BTN_BIT)

#define PDM_ID_BLINK_MODE   	    0x2
#define BLINK_MODE_SLOW		    0
#define BLINK_MODE_FAST		    1

uint8 blinkMode = BLINK_MODE_SLOW;

tsZLO_OnOffLightSwitchDevice sSwitch;






void storeBlinkMode(uint8 mode)
{
	blinkMode = mode;
	PDM_teStatus status = PDM_eSaveRecordData(PDM_ID_BLINK_MODE, &blinkMode, sizeof(blinkMode));
	DBG_vPrintf(TRUE, "Storing blink mode. Status %d, value %d\n", status, blinkMode);	
}

void restoreBlinkMode()
{
	uint16 readBytes;
	PDM_teStatus status = PDM_eReadDataFromRecord(PDM_ID_BLINK_MODE, &blinkMode, sizeof(blinkMode), &readBytes);

	DBG_vPrintf(TRUE, "Reading blink mode. Status %d, size %d, value %d\n", status, readBytes, blinkMode);	
}


ZTIMER_tsTimer timers[2 + BDB_ZTIMER_STORAGE];
uint8 blinkTimerHandle;
uint8 buttonScanTimerHandle;

typedef enum
{
	BUTTON_SHORT_PRESS,
	BUTTON_LONG_PRESS
} ButtonPressType;

ButtonPressType queue[3];
tszQueue queueHandle;

#define BDB_QUEUE_SIZE              3
#define MLME_QUEUE_SIZE             10
#define MCPS_QUEUE_SIZE             24
#define TIMER_QUEUE_SIZE            8
#define MCPS_DCFM_QUEUE_SIZE        5

extern PUBLIC tszQueue zps_msgMlmeDcfmInd;
extern PUBLIC tszQueue zps_msgMcpsDcfmInd;
extern PUBLIC tszQueue zps_TimeEvents;
extern PUBLIC tszQueue zps_msgMcpsDcfm;

PRIVATE MAC_tsMlmeVsDcfmInd asMacMlmeVsDcfmInd[MLME_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsDcfmInd asMacMcpsDcfmInd[MCPS_QUEUE_SIZE];
PRIVATE MAC_tsMcpsVsCfmData asMacMcpsDcfm[MCPS_DCFM_QUEUE_SIZE];
PRIVATE zps_tsTimeEvent asTimeEvent[TIMER_QUEUE_SIZE];
PRIVATE BDB_tsZpsAfEvent asBdbEvent[BDB_QUEUE_SIZE];

PRIVATE tszQueue APP_msgBdbEvents;



uint8 enabled = TRUE;

PUBLIC void blinkFunc(void *pvParam)
{
	ButtonPressType value;	
	if(ZQ_bQueueReceive(&queueHandle, (uint8*)&value))
	{
		DBG_vPrintf(TRUE, "Processing message in blink task\n");

		if(value == BUTTON_SHORT_PRESS)
		{
			blinkMode = (blinkMode == BLINK_MODE_FAST) ? BLINK_MODE_SLOW : BLINK_MODE_FAST;
			storeBlinkMode(blinkMode);
		}

		if(value == BUTTON_LONG_PRESS)
		{
			DBG_vPrintf(TRUE, "Stop Blinking\n");
			vAHI_DioSetOutput(BOARD_LED_PIN, 0);
			enabled = FALSE;
		}
	}

	if(enabled)
	{
		uint32 currentState = u32AHI_DioReadInput();
		vAHI_DioSetOutput(currentState^BOARD_LED_PIN, currentState&BOARD_LED_PIN);
	}

	ZTIMER_eStart(blinkTimerHandle, (blinkMode == BLINK_MODE_FAST) ? ZTIMER_TIME_MSEC(200) : ZTIMER_TIME_MSEC(1000));
}


PUBLIC void buttonScanFunc(void *pvParam)
{
	static int duration = 0;

	uint32 input = u32AHI_DioReadInput();
	bool btnState = (input & BOARD_BTN_PIN) == 0;

	if(btnState)
	{
		duration++;
		DBG_vPrintf(TRUE, "Button still pressed for %d ticks\n", duration);
	}
	else
	{
		// detect long press
		if(duration > 200)
		{
			DBG_vPrintf(TRUE, "Button released. Long press detected\n");
			ButtonPressType value = BUTTON_LONG_PRESS;
			ZQ_bQueueSend(&queueHandle, (uint8*)&value);
		}

		// detect short press
		else if(duration > 5)
		{
			DBG_vPrintf(TRUE, "Button released. Short press detected\n");
			ButtonPressType value = BUTTON_SHORT_PRESS;
			ZQ_bQueueSend(&queueHandle, &value);
		}

		duration = 0;
	}

	ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
}

extern "C" PUBLIC void vISR_SystemController(void)
{
    // clear pending DIO changed bits by reading register
    uint32 u32IOStatus = u32AHI_DioInterruptStatus();

    DBG_vPrintf(TRUE, "In vISR_SystemController\n");

    if(u32IOStatus & BOARD_BTN_PIN)
    {
        DBG_vPrintf(TRUE, "Button interrupt\n");
        enabled = TRUE;
        PWRM_vWakeInterruptCallback();
    }
}

PUBLIC void wakeCallBack(void)
{
    DBG_vPrintf(TRUE, "wakeCallBack()\n");
}


PRIVATE void APP_ZCL_cbGeneralCallback(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "APP_ZCL_cbGeneralCallback(): Processing event %d\n", psEvent->eEventType);

    switch (psEvent->eEventType)
    {

        case E_ZCL_CBET_UNHANDLED_EVENT:
            DBG_vPrintf(TRACE_ZCL, "EVT: Unhandled Event\r\n");
            break;

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, "EVT: Read attributes response\r\n");
            break;

        case E_ZCL_CBET_READ_REQUEST:
            DBG_vPrintf(TRACE_ZCL, "EVT: Read request\r\n");
            break;

        case E_ZCL_CBET_DEFAULT_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, "EVT: Default response\r\n");
            break;

        case E_ZCL_CBET_ERROR:
            DBG_vPrintf(TRACE_ZCL, "EVT: Error\r\n");
            break;

        case E_ZCL_CBET_TIMER:
            break;

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRACE_ZCL, "EVT: ZigBee\r\n");
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRACE_ZCL, "EP EVT: Custom\r\n");
            break;

        default:
            DBG_vPrintf(TRACE_ZCL, "Invalid event type (%d) in APP_ZCL_cbGeneralCallback\r\n", psEvent->eEventType);
            break;
    }
}

PRIVATE void APP_ZCL_cbEndpointCallback(tsZCL_CallBackEvent *psEvent)
{
    DBG_vPrintf(TRUE, "APP_ZCL_cbEndpointCallback(): Processing event %d\n", psEvent->eEventType);

    switch (psEvent->eEventType)
    {
        case E_ZCL_CBET_UNHANDLED_EVENT:

        case E_ZCL_CBET_READ_ATTRIBUTES_RESPONSE:

        case E_ZCL_CBET_READ_REQUEST:

        case E_ZCL_CBET_DEFAULT_RESPONSE:

        case E_ZCL_CBET_ERROR:

        case E_ZCL_CBET_TIMER:

        case E_ZCL_CBET_ZIGBEE_EVENT:
            DBG_vPrintf(TRACE_ZCL, "EP EVT:No action (evt type %d)\r\n", psEvent->eEventType);
            break;

        case E_ZCL_CBET_READ_INDIVIDUAL_ATTRIBUTE_RESPONSE:
            DBG_vPrintf(TRACE_ZCL, " Read Attrib Rsp %d %02x\n", psEvent->uMessage.sIndividualAttributeResponse.eAttributeStatus,
                *((uint8*)psEvent->uMessage.sIndividualAttributeResponse.pvAttributeData));
            break;

        case E_ZCL_CBET_CLUSTER_CUSTOM:
            DBG_vPrintf(TRACE_ZCL, "EP EVT: Custom %04x\r\n", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
            break;

        default:
            DBG_vPrintf(TRACE_ZCL, "EP EVT: Invalid event type (%d) in APP_ZCL_cbEndpointCallback\r\n", psEvent->eEventType);
            break;
    }
}


void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRUE,"ERROR: Extended status %x\n", eExtendedStatus);
}

PRIVATE void vHandleDiscoveryComplete(ZPS_tsAfNwkDiscoveryEvent * pEvent)
{
    DBG_vPrintf(TRUE, "Network Discovery Complete: status %d\n", pEvent->eStatus);
    DBG_vPrintf(TRUE, "    Network count: %d\n", pEvent->u8NetworkCount);
    DBG_vPrintf(TRUE, "    Selected network: %d\n", pEvent->u8SelectedNetwork);
    DBG_vPrintf(TRUE, "    Unscanned channels: %4x\n", pEvent->u32UnscannedChannels);

    for(uint8 i = 0; i < pEvent->u8NetworkCount; i++)
    {
        DBG_vPrintf(TRUE, "    Network %d\n", i);

        ZPS_tsNwkNetworkDescr * pNetwork = pEvent->psNwkDescriptors + i;

        DBG_vPrintf(TRUE, "        Extended PAN ID : %08x%08x\n", (uint32)((pNetwork->u64ExtPanId >> 32)&0xffffffff), (uint32)(pNetwork->u64ExtPanId & 0xffffffff));
        DBG_vPrintf(TRUE, "        Logical channel : %d\n", pNetwork->u8LogicalChan);
        DBG_vPrintf(TRUE, "        Stack Profile: %d\n", pNetwork->u8StackProfile);
        DBG_vPrintf(TRUE, "        ZigBee version: %d\n", pNetwork->u8ZigBeeVersion);
        DBG_vPrintf(TRUE, "        Permit Joining: %d\n", pNetwork->u8PermitJoining);
        DBG_vPrintf(TRUE, "        Router capacity: %d\n", pNetwork->u8RouterCapacity);
        DBG_vPrintf(TRUE, "        End device capacity: %d\n", pNetwork->u8EndDeviceCapacity);
    }


    // Check if there is a suitable network to join
    if(pEvent->u8SelectedNetwork == 0xff)
    {
        DBG_vPrintf(TRUE, "    No good network to join\n");
        return;
    }

    // Join the network
    ZPS_tsNwkNetworkDescr * pNetwork = pEvent->psNwkDescriptors + pEvent->u8SelectedNetwork;
    DBG_vPrintf(TRUE, "Network Discovery Complete: Joining network %08x%08x\n", (uint32)((pNetwork->u64ExtPanId >> 32)&0xffffffff), (uint32)(pNetwork->u64ExtPanId & 0xffffffff));
    ZPS_teStatus status = ZPS_eAplZdoJoinNetwork(pNetwork);
    DBG_vPrintf(TRUE, "Network Discovery Complete: ZPS_eAplZdoJoinNetwork() status %d\n", status);
}

PRIVATE void vHandleDataConfirm(ZPS_tsAfDataConfEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_CONFIRM: SrcEP=%d DstEP=%d DstAddr=%04x Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uDstAddr.u16Addr,
            pEvent->u8Status);
}

PRIVATE void vHandleDataAck(ZPS_tsAfDataAckEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_ACK: SrcEP=%d DrcEP=%d DstAddr=%04x Profile=%04x Cluster=%04x\n",
                pEvent->u8SrcEndpoint,
                pEvent->u8DstEndpoint,
                pEvent->u16DstAddr,
                pEvent->u16ProfileId,
                pEvent->u16ClusterId);
}

PRIVATE void vHandleJoinedAsRouter(ZPS_tsAfNwkJoinedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_JOINED_AS_ROUTER: Addr=%04x, rejoin=%d, secured rejoin=%d\n",
                pEvent->u16Addr,
                pEvent->bRejoin,
                pEvent->bSecuredRejoin);
}

PRIVATE void vHandleNwkStatusIndication(ZPS_tsAfNwkStatusIndEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_STATUS_INDICATION: Addr:%04x Status:%02x\n",
        pEvent->u16NwkAddr,
        pEvent->u8Status);
}

PRIVATE void vHandleRunningStackEvent(ZPS_tsAfEvent* psStackEvent)
{
    switch(psStackEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_CONFIRM:
            vHandleDataConfirm(&psStackEvent->uEvent.sApsDataConfirmEvent);
            break;

        case ZPS_EVENT_APS_DATA_ACK:
            vHandleDataAck(&psStackEvent->uEvent.sApsDataAckEvent);
            break;

        case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
            vHandleJoinedAsRouter(&psStackEvent->uEvent.sNwkJoinedEvent);
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            vHandleNwkStatusIndication(&psStackEvent->uEvent.sNwkStatusIndicationEvent);
            break;

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            vHandleDiscoveryComplete(&psStackEvent->uEvent.sNwkDiscoveryEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "Handle ZDO event: event type %d\n", psStackEvent->eType);
            break;
    }
}

PRIVATE void vAppHandleZdoEvents(BDB_tsZpsAfEvent *psZpsAfEvent)
{
    // Handle events depending on node state
    // if(sDeviceDesc.eNodeState == E_RUNNING)
        vHandleRunningStackEvent(&psZpsAfEvent->sStackEvent);
}


PRIVATE void vAppHandleAfEvent(BDB_tsZpsAfEvent *psZpsAfEvent)
{
//    if(psZpsAfEvent->u8EndPoint == app_u8GetDeviceEndpoint())
//    {
//        if((psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION) ||
//           (psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION))
//        {
//            DBG_vPrintf(TRACE_SWITCH_NODE, "Pass to ZCL\n");
//            APP_ZCL_vEventHandler(&psZpsAfEvent->sStackEvent);
//        }
//    }
//    else
    if(psZpsAfEvent->u8EndPoint == HELLOZIGBEE_ZDO_ENDPOINT)
    {
        // events for ep 0
        vAppHandleZdoEvents(psZpsAfEvent);
    }
    else
        DBG_vPrintf(TRUE, "AF event callback: endpoint %d, event %d\n", psZpsAfEvent->u8EndPoint, psZpsAfEvent->sStackEvent.eType);

    // Ensure Freeing of Apdus
    if(psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_DATA_INDICATION)
    {
        DBG_vPrintf(TRUE, "AF event callback: freeing up data event APDU\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsDataIndEvent.hAPduInst);
    }
    else if(psZpsAfEvent->sStackEvent.eType == ZPS_EVENT_APS_INTERPAN_DATA_INDICATION)
    {
        DBG_vPrintf(TRUE, "AF event callback: freeing up inter-PAN data event APDU\n");
        PDUM_eAPduFreeAPduInstance(psZpsAfEvent->sStackEvent.uEvent.sApsInterPanDataIndEvent.hAPduInst);
    }
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
            break;

        default:
            DBG_vPrintf(1, "BDB event callback: evt %d\n", psBdbEvent->eEventType);
            break;
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
    restoreBlinkMode();
    DBG_vPrintf(TRUE, "vAppMain(): PDM Capacity %d Occupancy %d\n",
            u8PDM_CalculateFileSystemCapacity(),
            u8PDM_GetFileSystemOccupancy() );


    // Initialize hardware
    DBG_vPrintf(TRUE, "vAppMain(): init GPIO...\n");
    vAHI_DioSetDirection(BOARD_BTN_PIN, BOARD_LED_PIN);
    vAHI_DioSetPullup(BOARD_BTN_PIN, 0);
    vAHI_DioInterruptEdge(0, BOARD_BTN_PIN);
    vAHI_DioInterruptEnable(BOARD_BTN_PIN, 0);

    // Initialize power manager and sleep mode
    DBG_vPrintf(TRUE, "vAppMain(): init PWRM...\n");
    PWRM_vInit(E_AHI_SLEEP_DEEP);

    // PDU Manager initialization
    DBG_vPrintf(TRUE, "vAppMain(): init PDUM...\n");
    PDUM_vInit();

    // Init and start timers
    DBG_vPrintf(TRUE, "vAppMain(): init software timers...\n");
    ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));
    ZTIMER_eOpen(&blinkTimerHandle, blinkFunc, NULL, ZTIMER_FLAG_ALLOW_SLEEP);
    ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));
    ZTIMER_eOpen(&buttonScanTimerHandle, buttonScanFunc, NULL, ZTIMER_FLAG_ALLOW_SLEEP);
    ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));

    // Initialize queues
    DBG_vPrintf(TRUE, "vAppMain(): init software queues...\n");
    ZQ_vQueueCreate(&queueHandle, 3, sizeof(ButtonPressType), (uint8*)queue);

    // Initialize ZigBee stack queues
    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd, MLME_QUEUE_SIZE, sizeof(MAC_tsMlmeVsDcfmInd), (uint8*)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd, MCPS_QUEUE_SIZE, sizeof(MAC_tsMcpsVsDcfmInd), (uint8*)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_TimeEvents, TIMER_QUEUE_SIZE, sizeof(zps_tsTimeEvent), (uint8*)asTimeEvent);
    ZQ_vQueueCreate(&zps_msgMcpsDcfm, MCPS_DCFM_QUEUE_SIZE,	sizeof(MAC_tsMcpsVsCfmData), (uint8*)asMacMcpsDcfm);
    ZQ_vQueueCreate(&APP_msgBdbEvents, BDB_QUEUE_SIZE, sizeof(BDB_tsZpsAfEvent), (uint8*)asBdbEvent);


    // Set up a status callback
    DBG_vPrintf(TRUE, "vAppMain(): init extended status callback...\n");
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    DBG_vPrintf(TRUE, "vAppMain(): init Zigbee Class Library (ZCL)...  ");
    ZPS_teStatus status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    DBG_vPrintf(TRUE, "eZCL_Initialise() status %d\n", status);

    DBG_vPrintf(TRUE, "vAppMain(): register On/Off endpoint...  ");
    status = eZLO_RegisterOnOffLightSwitchEndPoint(HELLOZIGBEE_SWITCH_ENDPOINT, &APP_ZCL_cbEndpointCallback, &sSwitch);
    DBG_vPrintf(TRUE, "eApp_ZCL_RegisterEndpoint() status %d\n", status);

    // Initialise Application Framework stack
    DBG_vPrintf(TRUE, "vAppMain(): init Application Framework (AF)... ");
    status = ZPS_eAplAfInit();
    DBG_vPrintf(TRUE, "ZPS_eAplAfInit() status %d\n", status);

    // Initialize Base Class Behavior
    DBG_vPrintf(TRUE, "vAppMain(): initialize base device behavior...\n");
    BDB_tsInitArgs sInitArgs;
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
    BDB_vInit(&sInitArgs);

    //vAPP_ZCL_DeviceSpecific_Init();

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

        //APP_taskSwitch();

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

    // Set the wake condition on falling edge of the button pin
    vAHI_DioWakeEdge(0, BOARD_BTN_PIN);
    vAHI_DioWakeEnable(BOARD_BTN_PIN, 0);
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

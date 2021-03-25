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
    //#include "MiniMac.h"
    #include "zcl.h"
    #include "zps_apl.h"
    #include "zps_apl_af.h"

    // Zigbee cluster includes
    #include "on_off_light_switch.h"
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



ZTIMER_tsTimer timers[2];
uint8 blinkTimerHandle;
uint8 buttonScanTimerHandle;

typedef enum
{
	BUTTON_SHORT_PRESS,
	BUTTON_LONG_PRESS
} ButtonPressType;

ButtonPressType queue[3];
tszQueue queueHandle;

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

extern "C" PUBLIC void vAppMain(void)
{
    // Initialize the hardware
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();

    // Initialize UART
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    DBG_vPrintf(TRUE, "vAppMain()\n");

    // Restore blink mode from EEPROM
    PDM_eInitialise(0);
    restoreBlinkMode();

    // Initialize hardware
    vAHI_DioSetDirection(BOARD_BTN_PIN, BOARD_LED_PIN);
    vAHI_DioSetPullup(BOARD_BTN_PIN, 0);
    vAHI_DioInterruptEdge(0, BOARD_BTN_PIN);
    vAHI_DioInterruptEnable(BOARD_BTN_PIN, 0);

    // Init and start timers
    ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));
    ZTIMER_eOpen(&blinkTimerHandle, blinkFunc, NULL, ZTIMER_FLAG_ALLOW_SLEEP);
    ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));
    ZTIMER_eOpen(&buttonScanTimerHandle, buttonScanFunc, NULL, ZTIMER_FLAG_ALLOW_SLEEP);
    ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));

    // Initialize queue
    ZQ_vQueueCreate(&queueHandle, 3, sizeof(ButtonPressType), (uint8*)queue);

    // Let the device go to sleep if there is nothing to do
    PWRM_vInit(E_AHI_SLEEP_DEEP);

    // PDU Manager initialization
    PDUM_vInit();

    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    // Initialise ZBPro stack
    ZPS_teStatus status = ZPS_eAplAfInit();
    DBG_vPrintf(TRUE, "ZPS_eAplAfInit() status %d\n", status);
    status = eZCL_Initialise(&APP_ZCL_cbGeneralCallback, apduZCL);
    DBG_vPrintf(TRUE, "eZCL_Initialise() status %d\n", status);
    status = eZLO_RegisterOnOffLightSwitchEndPoint(HELLOZIGBEE_SWITCH_ENDPOINT, &APP_ZCL_cbEndpointCallback, &sSwitch);
    DBG_vPrintf(TRUE, "eApp_ZCL_RegisterEndpoint() status %d\n", status);
    DBG_vPrintf(TRACE_ZCL, "Chan Mask %08x\n", ZPS_psAplAibGetAib()->pau32ApsChannelMask[0]);

    //vAPP_ZCL_DeviceSpecific_Init();


    while(1)
    {
        ZTIMER_vTask();

        vAHI_WatchdogRestart();

        if(enabled == FALSE)
        {
            DBG_vPrintf(TRUE, "Scheduling sleep\n");
            PWRM_vManagePower();
        }
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

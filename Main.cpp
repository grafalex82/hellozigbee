extern "C"
{
    #include "AppHardwareApi.h"
    #include "dbg.h"
    #include "dbg_uart.h"
    #include "portmacro.h"
    #include "pwrm.h"
    #include "PDM.h"

    // Local configuration and generated files
    #include "pdum_gen.h"
    #include "zps_gen.h"
}

#include "Queue.h"
#include "DeferredExecutor.h"
#include "ButtonsTask.h"
#include "AppQueue.h"
#include "SwitchEndpoint.h"
#include "EndpointManager.h"
#include "BasicClusterEndpoint.h"
#include "ZigbeeDevice.h"

DeferredExecutor deferredExecutor;

// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}


ZTIMER_tsTimer timers[4 + BDB_ZTIMER_STORAGE];


struct Context
{
    BasicClusterEndpoint basicEndpoint;
    SwitchEndpoint switch1;
};


extern "C" void __cxa_pure_virtual(void) __attribute__((__noreturn__));
extern "C" void __cxa_deleted_virtual(void) __attribute__((__noreturn__));

void __cxa_pure_virtual(void)
{
  DBG_vPrintf(TRUE, "!!!!!!! Pure virtual function call.\n");
  while (1)
    ;
}


extern "C" PUBLIC void vISR_SystemController(void)
{
    // clear pending DIO changed bits by reading register
    uint8 wakeStatus = u8AHI_WakeTimerFiredStatus();
    uint32 dioStatus = u32AHI_DioInterruptStatus();

    DBG_vPrintf(TRUE, "In vISR_SystemController\n");

    if(ButtonsTask::getInstance()->handleDioInterrupt(dioStatus))
    {
        DBG_vPrintf(TRUE, "=-=-=- Button interrupt dioStatus=%04x\n", dioStatus);
        PWRM_vWakeInterruptCallback();
    }

    if(wakeStatus & E_AHI_WAKE_TIMER_MASK_1)
    {
        DBG_vPrintf(TRUE, "=-=-=- Wake Timer Interrupt\n");
        PWRM_vWakeInterruptCallback();
    }
}

void vfExtendedStatusCallBack (ZPS_teExtendedStatus eExtendedStatus)
{
    DBG_vPrintf(TRUE,"ERROR: Extended status %x\n", eExtendedStatus);
}

#if 0
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
#endif //0

PUBLIC void wakeCallBack(void)
{
    DBG_vPrintf(TRUE, "=-=-=- wakeCallBack()\n");
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
            ZigbeeDevice::getInstance()->joinOrLeaveNetwork();
        }
    }

    if(ButtonsTask::getInstance()->canSleep() &&
       ZigbeeDevice::getInstance()->canSleep())
    {
        static pwrm_tsWakeTimerEvent wakeStruct;
        PWRM_teStatus status = PWRM_eScheduleActivity(&wakeStruct, 15 * 32000, wakeCallBack);
        if(status != PWRM_E_TIMER_RUNNING)
            DBG_vPrintf(TRUE, "=-=-=- Scheduling enter sleep mode... status=%d\n", status);
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
    PWRM_vInit(E_AHI_SLEEP_OSCON_RAMON);

    // PDU Manager initialization
    DBG_vPrintf(TRUE, "vAppMain(): init PDUM...\n");
    PDUM_vInit();

    // Init timers
    DBG_vPrintf(TRUE, "vAppMain(): init software timers...\n");
    ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));

    // Init tasks
    DBG_vPrintf(TRUE, "vAppMain(): init tasks...\n");
    ButtonsTask::getInstance();

    // Initialize application queue
    DBG_vPrintf(TRUE, "vAppMain(): init software queues...\n");
    appEventQueue.init();

    // Initialize deferred executor
    DBG_vPrintf(TRUE, "vAppMain(): Initialize deferred executor...\n");
    deferredExecutor.init();

    // Set up a status callback
    DBG_vPrintf(TRUE, "vAppMain(): init extended status callback...\n");
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    DBG_vPrintf(TRUE, "vAppMain(): Registering endpoint objects\n");
    Context context;
    EndpointManager::getInstance()->registerEndpoint(HELLOENDDEVICE_BASIC_ENDPOINT, &context.basicEndpoint);
    EndpointManager::getInstance()->registerEndpoint(HELLOENDDEVICE_SWITCH_ENDPOINT, &context.switch1);

    // Start ZigbeeDevice and rejoin the network (if was joined)
    ZigbeeDevice::getInstance()->rejoinNetwork();

    DBG_vPrintf(TRUE, "vAppMain(): Starting the main loop\n");
    while(1)
    {
        zps_taskZPS();

        bdb_taskBDB();

        ZTIMER_vTask();

        APP_vTaskSwitch(&context);

        vAHI_WatchdogRestart();

        PWRM_vManagePower();
    }
}

static PWRM_DECLARE_CALLBACK_DESCRIPTOR(PreSleep);
static PWRM_DECLARE_CALLBACK_DESCRIPTOR(Wakeup);

PWRM_CALLBACK(PreSleep)
{
    DBG_vPrintf(TRUE, "Going to sleep..\n\n");

    // Save the MAC settings (will get lost though if we don't preserve RAM)
    vAppApiSaveMacSettings();

    // Put ZTimer module to sleep (stop tick timer)
    ZTIMER_vSleep();

    // Disable UART (if enabled)
    DBG_vUartFlush();
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
    DBG_vPrintf(TRUE, "\nWaking...\n");
    DBG_vUartFlush();

    // Re-initialize hardware and interrupts
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();

    // Restore Mac settings (turns radio on)
    vMAC_RestoreSettings();

    // Wake the timers
    ZTIMER_vWake();

    // Poll the parent router for zigbee messages
    ZigbeeDevice::getInstance()->handleWakeUp();
}

extern "C" void vAppRegisterPWRMCallbacks(void)
{
    PWRM_vRegisterPreSleepCallback(PreSleep);
    PWRM_vRegisterWakeupCallback(Wakeup);	
}

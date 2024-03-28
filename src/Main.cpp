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
#include "ButtonsTask.h"
#include "SwitchEndpoint.h"
#include "EndpointManager.h"
#include "BasicClusterEndpoint.h"
#include "ZigbeeDevice.h"
#include "ZCLTimer.h"
#include "LEDTask.h"
#include "BlinkTask.h"
#include "RelayTask.h"
#include "DumpFunctions.h"
#include "DebugInput.h"


// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}


// 7 timers are:
// - 1 in ButtonTask
// - 1 in LEDTask
// - 1 in RelayTask
// - 1 in Heartbeat BlinkTask
// - 1 in PollTask
// - 1 is ZCL timer
// Note: if not enough space in this timers array, some of the functions (e.g. network joining) may not work properly
ZTIMER_tsTimer timers[7 + BDB_ZTIMER_STORAGE];

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

PUBLIC void wakeCallBack(void)
{
    DBG_vPrintf(TRUE, "=-=-=- wakeCallBack()\n");
}

PRIVATE void scheduleSleep()
{
    if(ButtonsTask::getInstance()->canSleep() &&
       ZigbeeDevice::getInstance()->canSleep() &&
       LEDTask::getInstance()->canSleep() &&
       RelayTask::getInstance()->canSleep())
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
    vAHI_UartSetRTSCTS(E_AHI_UART_0, FALSE);
    DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

    // Print welcome message
    DBG_vPrintf(TRUE, "\n-------------------------------------------------------------\n");
    DBG_vPrintf(TRUE, "Initializing Hello Zigbee Platform for target board '%s'\n", CLD_BAS_MODEL_ID_STR);
    DBG_vPrintf(TRUE, "-------------------------------------------------------------\n\n");

    // Initialize PDM
    DBG_vPrintf(TRUE, "vAppMain(): init PDM...  ");
    PDM_eInitialise(0);
    DBG_vPrintf(TRUE, "PDM Capacity %d Occupancy %d\n",
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
    DBG_vPrintf(TRUE, "vAppMain(): init periodic tasks...\n");
    ZCLTimer::getInstance()->start();
    ButtonsTask::getInstance()->start();
    LEDTask::getInstance();
    RelayTask::getInstance();

    // Initialize the heartbeat LED (if there is one)
#ifdef HEARTBEAT_LED_MASK
    BlinkTask blinkTask;
    blinkTask.init(HEARTBEAT_LED_MASK);
#endif

    // Set up a status callback
    DBG_vPrintf(TRUE, "vAppMain(): init extended status callback...\n");
    ZPS_vExtendedStatusSetCallback(vfExtendedStatusCallBack);

    // Register endpoints and assign buttons for them
    DBG_vPrintf(TRUE, "vAppMain(): Registering endpoint objects\n");
    BasicClusterEndpoint basicEndpoint;
    EndpointManager::getInstance()->registerEndpoint(BASIC_ENDPOINT, &basicEndpoint);

    SwitchEndpoint switch1;
    switch1.setConfiguration(SWITCH1_BTN_MASK);
    EndpointManager::getInstance()->registerEndpoint(SWITCH1_ENDPOINT, &switch1);

#ifdef SWITCH2_BTN_PIN
    SwitchEndpoint switch2;
    switch2.setConfiguration(SWITCH2_BTN_MASK);
    EndpointManager::getInstance()->registerEndpoint(SWITCH2_ENDPOINT, &switch2);

    switch1.setInterlockBuddy(&switch2);
    switch2.setInterlockBuddy(&switch1);

    SwitchEndpoint switchBoth;
    switchBoth.setConfiguration(SWITCH1_BTN_MASK | SWITCH2_BTN_MASK, true);
    EndpointManager::getInstance()->registerEndpoint(SWITCHB_ENDPOINT, &switchBoth);
#endif

    // Init the ZigbeeDevice, AF, BDB, and other network stuff
    ZigbeeDevice::getInstance();

    // Print Initialization finished message
    DBG_vPrintf(TRUE, "\n---------------------------------------------------\n");
    DBG_vPrintf(TRUE, "Initialization of the Hello Zigbee Platform Finished\n");
    DBG_vPrintf(TRUE, "---------------------------------------------------\n\n");

    // Start ZigbeeDevice and rejoin the network (if was joined)
    ZigbeeDevice::getInstance()->rejoinNetwork();

    DBG_vPrintf(TRUE, "\nvAppMain(): Starting the main loop\n");
    while(1)
    {
        // Run Zigbee stack stuff
        zps_taskZPS();
        bdb_taskBDB();

        // Process all periodic tasks
        ZTIMER_vTask();

        // Process all incoming debug input
        DebugInput::getInstance().handleInput();

        // Schedule sleep, if no activities are running. Reset the watchdog timer.
        scheduleSleep();
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

    // Restore Mac settings (turns radio on)
    vMAC_RestoreSettings();

    // Re-initialize hardware and interrupts
    TARGET_INITIALISE();
    SET_IPL(0);
    portENABLE_INTERRUPTS();

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

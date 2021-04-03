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

// Hidden funcctions (exported from the library, but not mentioned in header files)
extern "C"
{
    PUBLIC uint8 u8PDM_CalculateFileSystemCapacity(void);
    PUBLIC uint8 u8PDM_GetFileSystemOccupancy(void);
    extern void zps_taskZPS(void);
}



tsZCL_AttributeReportingConfigurationRecord switchReports[ZCL_NUMBER_OF_REPORTS] =
{
    {
        0,                                          // Direction: Server to Client
        E_ZCL_BOOL,                                 // Attribute type
        E_CLD_ONOFF_ATTR_ID_ONOFF,                  // Attribute ID
        ZLO_MIN_REPORT_INTERVAL,                    // Min reporting interval
        ZLO_MAX_REPORT_INTERVAL,                    // Max reporting interval
        REPORTS_OF_ATTRIBUTE_NOT_SUBJECT_TO_TIMEOUT,   // Attribute value does not expire
        {
            0                                       // Minimum reportable change
        }
    }
};


#define BOARD_LED_BIT               (17)
#define BOARD_LED_PIN               (1UL << BOARD_LED_BIT)

#define BOARD_BTN_BIT               (1)
#define BOARD_BTN_PIN               (1UL << BOARD_BTN_BIT)

#define PDM_ID_BLINK_MODE   	    0x2

tsZLO_OnOffLightDevice sSwitch;


void storeBlinkMode(bool_t blinkMode)
{
    PDM_teStatus status = PDM_eSaveRecordData(PDM_ID_BLINK_MODE, &blinkMode, sizeof(blinkMode));
    DBG_vPrintf(TRUE, "Storing blink mode. Status %d, value %d\n", status, blinkMode);
}

void restoreBlinkMode()
{
	uint16 readBytes;
    PDM_teStatus status1 = PDM_eReadDataFromRecord(PDM_ID_BLINK_MODE,
                                                   &sSwitch.sOnOffServerCluster.bOnOff,
                                                   sizeof(zbool),
                                                   &readBytes);
    DBG_vPrintf(TRUE, "Reading blink mode. Status %d, size %d, value %d\n", status1, readBytes, sSwitch.sOnOffServerCluster.bOnOff);
}


ZTIMER_tsTimer timers[3 + BDB_ZTIMER_STORAGE];
uint8 blinkTimerHandle;
uint8 buttonScanTimerHandle;
uint8 runLaterTimerHandle;

typedef enum
{
	BUTTON_SHORT_PRESS,
	BUTTON_LONG_PRESS
} ButtonPressType;

ButtonPressType buttonQueue[3];
tszQueue buttonQueueHandle;


#define RUN_LATER_QUEUE_SIZE 20
typedef void (*runLaterCallback)(uint8);
uint32 runLaterDelayQueue[RUN_LATER_QUEUE_SIZE];
uint8 runLaterParamsQueue[RUN_LATER_QUEUE_SIZE];
runLaterCallback runLaterCallbackQueue[RUN_LATER_QUEUE_SIZE];

PRIVATE tszQueue runLaterDelayQueueHandle;
PRIVATE tszQueue runLaterCallbacksQueueHandle;
PRIVATE tszQueue runLaterParamsQueueHandle;


void runLater(uint32 delay, runLaterCallback cb, uint8 param)
{
    ZQ_bQueueSend(&runLaterDelayQueueHandle, (uint8*)&delay);
    ZQ_bQueueSend(&runLaterCallbacksQueueHandle, (uint8*)&cb);
    ZQ_bQueueSend(&runLaterParamsQueueHandle, (uint8*)&param);
}

PUBLIC void runLaterFunc(void *pvParam)
{
    DBG_vPrintf(TRUE, "runLaterFunc()\n");

    runLaterCallback cb;
    ZQ_bQueueReceive(&runLaterCallbacksQueueHandle, (uint8*)&cb);
    uint8 param;
    ZQ_bQueueReceive(&runLaterParamsQueueHandle, (uint8*)&param);
    (*cb)(param);
}


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



PUBLIC void blinkFunc(void *pvParam)
{
    ZTIMER_eStart(blinkTimerHandle, sSwitch.sOnOffServerCluster.bOnOff ? ZTIMER_TIME_MSEC(200) : ZTIMER_TIME_MSEC(1000));
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
            ZQ_bQueueSend(&buttonQueueHandle, (uint8*)&value);
        }

        // detect short press
        else if(duration > 5)
        {
            DBG_vPrintf(TRUE, "Button released. Short press detected\n");
            ButtonPressType value = BUTTON_SHORT_PRESS;
            ZQ_bQueueSend(&buttonQueueHandle, &value);
        }

        duration = 0;
    }

    ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
}

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

PRIVATE void vDumpZclReadRequest(tsZCL_CallBackEvent *psEvent)
{
    // Read command header
    tsZCL_HeaderParams headerParams;
    uint16 inputOffset = u16ZCL_ReadCommandHeader(psEvent->pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
                                              &headerParams);

    // read input attribute Id
    uint16 attributeId;
    inputOffset += u16ZCL_APduInstanceReadNBO(psEvent->pZPSevent->uEvent.sApsDataIndEvent.hAPduInst,
                                              inputOffset,
                                              E_ZCL_ATTRIBUTE_ID,
                                              &attributeId);


    DBG_vPrintf(TRUE, "ZCL Read Attribute: EP=%d Cluster=%04x Command=%02x Attr=%04x\n",
                psEvent->u8EndPoint,
                psEvent->pZPSevent->uEvent.sApsDataIndEvent.u16ClusterId,
                headerParams.u8CommandIdentifier,
                attributeId);
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
            DBG_vPrintf(TRUE, "ZCL Endpoint Callback: Custom %04x\r\n", psEvent->uMessage.sClusterCustomMessage.u16ClusterId);
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
#endif

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

PRIVATE void vDumpDiscoveryCompleteEvent(ZPS_tsAfNwkDiscoveryEvent * pEvent)
{
    DBG_vPrintf(TRUE, "Network Discovery Complete: status %02x\n", pEvent->eStatus);
    DBG_vPrintf(TRUE, "    Network count: %d\n", pEvent->u8NetworkCount);
    DBG_vPrintf(TRUE, "    Selected network: %d\n", pEvent->u8SelectedNetwork);
    DBG_vPrintf(TRUE, "    Unscanned channels: %4x\n", pEvent->u32UnscannedChannels);

    for(uint8 i = 0; i < pEvent->u8NetworkCount; i++)
    {
        DBG_vPrintf(TRUE, "    Network %d\n", i);

        ZPS_tsNwkNetworkDescr * pNetwork = pEvent->psNwkDescriptors + i;

        DBG_vPrintf(TRUE, "        Extended PAN ID : %016llx\n", pNetwork->u64ExtPanId);
        DBG_vPrintf(TRUE, "        Logical channel : %d\n", pNetwork->u8LogicalChan);
        DBG_vPrintf(TRUE, "        Stack Profile: %d\n", pNetwork->u8StackProfile);
        DBG_vPrintf(TRUE, "        ZigBee version: %d\n", pNetwork->u8ZigBeeVersion);
        DBG_vPrintf(TRUE, "        Permit Joining: %d\n", pNetwork->u8PermitJoining);
        DBG_vPrintf(TRUE, "        Router capacity: %d\n", pNetwork->u8RouterCapacity);
        DBG_vPrintf(TRUE, "        End device capacity: %d\n", pNetwork->u8EndDeviceCapacity);
    }
}


PRIVATE void vDumpDataIndicationEvent(ZPS_tsAfDataIndEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_INDICATION: SrcEP=%d DstEP=%d SrcAddr=%04x Cluster=%04x Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uSrcAddress.u16Addr,
            pEvent->u16ClusterId,
            pEvent->eStatus);
}


PRIVATE void vDumpDataConfirmEvent(ZPS_tsAfDataConfEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_CONFIRM: SrcEP=%d DstEP=%d DstAddr=%04x Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uDstAddr.u16Addr,
            pEvent->u8Status);
}

PRIVATE void vDumpDataAckEvent(ZPS_tsAfDataAckEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_ACK: SrcEP=%d DrcEP=%d DstAddr=%04x Profile=%04x Cluster=%04x\n",
                pEvent->u8SrcEndpoint,
                pEvent->u8DstEndpoint,
                pEvent->u16DstAddr,
                pEvent->u16ProfileId,
                pEvent->u16ClusterId);
}

PRIVATE void vDumpJoinedAsRouterEvent(ZPS_tsAfNwkJoinedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_JOINED_AS_ROUTER: Addr=%04x, rejoin=%d, secured rejoin=%d\n",
                pEvent->u16Addr,
                pEvent->bRejoin,
                pEvent->bSecuredRejoin);
}

PRIVATE void vDumpNwkStatusIndicationEvent(ZPS_tsAfNwkStatusIndEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_STATUS_INDICATION: Addr:%04x Status:%02x\n",
        pEvent->u16NwkAddr,
        pEvent->u8Status);
}

PRIVATE void vDumpNwkFailedToJoinEvent(ZPS_tsAfNwkJoinFailedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_FAILED_TO_JOIN: Status: %02x Rejoin:%02x\n",
        pEvent->u8Status,
        pEvent->bRejoin);
}

PRIVATE void vDumpAfEvent(ZPS_tsAfEvent* psStackEvent)
{
    switch(psStackEvent->eType)
    {
        case ZPS_EVENT_APS_DATA_INDICATION:
            vDumpDataIndicationEvent(&psStackEvent->uEvent.sApsDataIndEvent);
            break;

        case ZPS_EVENT_APS_DATA_CONFIRM:
            vDumpDataConfirmEvent(&psStackEvent->uEvent.sApsDataConfirmEvent);
            break;

        case ZPS_EVENT_APS_DATA_ACK:
            vDumpDataAckEvent(&psStackEvent->uEvent.sApsDataAckEvent);
            break;

        case ZPS_EVENT_NWK_JOINED_AS_ROUTER:
            vDumpJoinedAsRouterEvent(&psStackEvent->uEvent.sNwkJoinedEvent);
            break;

        case ZPS_EVENT_NWK_STATUS_INDICATION:
            vDumpNwkStatusIndicationEvent(&psStackEvent->uEvent.sNwkStatusIndicationEvent);
            break;

        case ZPS_EVENT_NWK_FAILED_TO_JOIN:
            vDumpNwkFailedToJoinEvent(&psStackEvent->uEvent.sNwkJoinFailedEvent);
            break;

        case ZPS_EVENT_NWK_DISCOVERY_COMPLETE:
            vDumpDiscoveryCompleteEvent(&psStackEvent->uEvent.sNwkDiscoveryEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "Unknown Zigbee stack event: event type %d\n", psStackEvent->eType);
            break;
    }
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

                runLater(1000, vSendSimpleDescriptorReq, ep);
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

            //runLater(15000, vGetCoordinatorEndpoints, 0);

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
    ButtonPressType value;
    if(ZQ_bQueueReceive(&buttonQueueHandle, (uint8*)&value))
    {
        DBG_vPrintf(TRUE, "Processing button message %d\n", value);

        if(value == BUTTON_SHORT_PRESS)
        {
            PDUM_thAPduInstance myPDUM_thAPduInstance = hZCL_AllocateAPduInstance();

            // Destination address - 0x0000 (coordinator)
            tsZCL_Address addr;
            addr.uAddress.u16DestinationAddress = 0x0000;
            addr.eAddressMode = E_ZCL_AM_SHORT;

            DBG_vPrintf(TRUE, "Reporing attribute... ", value);
            teZCL_Status status = eZCL_ReportAttribute(&addr,
                                                       GENERAL_CLUSTER_ID_ONOFF,
                                                       E_CLD_ONOFF_ATTR_ID_ONOFF,
                                                       HELLOZIGBEE_SWITCH_ENDPOINT,
                                                       1,
                                                       myPDUM_thAPduInstance);
            DBG_vPrintf(TRUE, "status: %02x\n", status);

            PDUM_eAPduFreeAPduInstance(myPDUM_thAPduInstance);
        }

        if(value == BUTTON_LONG_PRESS)
        {
            // TODO: add network join here
        }
    }
}

PRIVATE void vRegisterEndPointAttributes(void)
{
    DBG_vPrintf(TRUE, "Register endpoint attributes:\n");

    for(uint8 i=0; i<ZCL_NUMBER_OF_REPORTS; i++)
    {
        teZCL_Status status = eZCL_CreateLocalReport( HELLOZIGBEE_SWITCH_ENDPOINT,      // Endpoint ID
                                                        GENERAL_CLUSTER_ID_ONOFF,       // Cluster ID
                                                        FALSE,                          // Manufacturer specific
                                                        TRUE,                           // Is server attribute?
                                                        switchReports + i);             // report record
        DBG_vPrintf(TRUE, "    Register attribute %04x status %02x\n", switchReports[i].u16AttributeEnum, status);


        status = eZCL_SetReportableFlag(HELLOZIGBEE_SWITCH_ENDPOINT,    // Endpoint ID
                                        GENERAL_CLUSTER_ID_ONOFF,       // Cluster ID
                                        TRUE,                           // Is server attribute?
                                        FALSE,                          // Manufacturer specific
                                        switchReports[i].u16AttributeEnum   // Attribute ID
                                        );

        DBG_vPrintf(TRUE, "    Attribute reportable flag set %02x\n", status);
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


    // Initialize hardware
    DBG_vPrintf(TRUE, "vAppMain(): init GPIO...\n");
    vAHI_DioSetDirection(BOARD_BTN_PIN, BOARD_LED_PIN);
    vAHI_DioSetPullup(BOARD_BTN_PIN, 0);

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
    ZTIMER_eOpen(&runLaterTimerHandle, runLaterFunc, NULL, ZTIMER_FLAG_ALLOW_SLEEP);

    // Initialize queues
    DBG_vPrintf(TRUE, "vAppMain(): init software queues...\n");
    ZQ_vQueueCreate(&buttonQueueHandle, 3, sizeof(ButtonPressType), (uint8*)buttonQueue);

    // Initialize ZigBee stack queues
    ZQ_vQueueCreate(&zps_msgMlmeDcfmInd, MLME_QUEUE_SIZE, sizeof(MAC_tsMlmeVsDcfmInd), (uint8*)asMacMlmeVsDcfmInd);
    ZQ_vQueueCreate(&zps_msgMcpsDcfmInd, MCPS_QUEUE_SIZE, sizeof(MAC_tsMcpsVsDcfmInd), (uint8*)asMacMcpsDcfmInd);
    ZQ_vQueueCreate(&zps_TimeEvents, TIMER_QUEUE_SIZE, sizeof(zps_tsTimeEvent), (uint8*)asTimeEvent);
    ZQ_vQueueCreate(&zps_msgMcpsDcfm, MCPS_DCFM_QUEUE_SIZE,	sizeof(MAC_tsMcpsVsCfmData), (uint8*)asMacMcpsDcfm);
    ZQ_vQueueCreate(&APP_msgBdbEvents, BDB_QUEUE_SIZE, sizeof(BDB_tsZpsAfEvent), (uint8*)asBdbEvent);
    ZQ_vQueueCreate(&runLaterDelayQueueHandle, RUN_LATER_QUEUE_SIZE, sizeof(uint32), (uint8*)runLaterDelayQueue);
    ZQ_vQueueCreate(&runLaterCallbacksQueueHandle, RUN_LATER_QUEUE_SIZE, sizeof(uint32), (uint8*)runLaterCallbackQueue);
    ZQ_vQueueCreate(&runLaterParamsQueueHandle, RUN_LATER_QUEUE_SIZE, sizeof(uint8), (uint8*)runLaterParamsQueue);

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

    // Register attributes
    vRegisterEndPointAttributes();

    // Set the initial value
    restoreBlinkMode();

    // Initialise Application Framework stack
    DBG_vPrintf(TRUE, "vAppMain(): init Application Framework (AF)... ");
    status = ZPS_eAplAfInit();
    DBG_vPrintf(TRUE, "ZPS_eAplAfInit() status %d\n", status);

    // Initialize Base Class Behavior
    DBG_vPrintf(TRUE, "vAppMain(): initialize base device behavior...\n");
    BDB_tsInitArgs sInitArgs;
    sInitArgs.hBdbEventsMsgQ = &APP_msgBdbEvents;
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

        uint32 runLaterDelay;
        if(ZTIMER_eGetState(runLaterTimerHandle) != E_ZTIMER_STATE_RUNNING &&
           ZQ_bQueueReceive(&runLaterDelayQueueHandle, (uint8*)&runLaterDelay))
        {
            DBG_vPrintf(TRUE, "Scheduing next runLater call in %d ms\n", runLaterDelay);
            ZTIMER_eStop(runLaterTimerHandle);
            ZTIMER_eStart(runLaterTimerHandle, runLaterDelay);
        }

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

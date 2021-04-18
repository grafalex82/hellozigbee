#include "DumpFunctions.h"

extern "C"
{
    #include "dbg.h"
    #include "zcl_customcommand.h"

    #include "bdb_api.h"
    // work around of a bug in appZpsBeaconHandler.h that does not have a closing } for its extern "C" statement
    }
}

void vDumpZclReadRequest(tsZCL_CallBackEvent *psEvent)
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

extern "C" void vDumpDiscoveryCompleteEvent(ZPS_tsAfNwkDiscoveryEvent * pEvent)
{
    DBG_vPrintf(TRUE, "Network Discovery Complete: status 0x%02x\n", pEvent->eStatus);
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

void vDumpDataIndicationEvent(ZPS_tsAfDataIndEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_INDICATION: SrcEP=%d DstEP=%d SrcAddr=%04x Cluster=%04x Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uSrcAddress.u16Addr,
            pEvent->u16ClusterId,
            pEvent->eStatus);
}

void vDumpDataConfirmEvent(ZPS_tsAfDataConfEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_CONFIRM: SrcEP=%d DstEP=%d DstAddr=%04x Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uDstAddr.u16Addr,
            pEvent->u8Status);
}

void vDumpDataAckEvent(ZPS_tsAfDataAckEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_ACK: SrcEP=%d DrcEP=%d DstAddr=%04x Profile=%04x Cluster=%04x\n",
                pEvent->u8SrcEndpoint,
                pEvent->u8DstEndpoint,
                pEvent->u16DstAddr,
                pEvent->u16ProfileId,
                pEvent->u16ClusterId);
}

void vDumpJoinedAsRouterEvent(ZPS_tsAfNwkJoinedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_JOINED_AS_ROUTER: Addr=%04x, rejoin=%d, secured rejoin=%d\n",
                pEvent->u16Addr,
                pEvent->bRejoin,
                pEvent->bSecuredRejoin);
}

void vDumpNwkStatusIndicationEvent(ZPS_tsAfNwkStatusIndEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_STATUS_INDICATION: Addr:%04x Status:%02x\n",
        pEvent->u16NwkAddr,
        pEvent->u8Status);
}

void vDumpNwkFailedToJoinEvent(ZPS_tsAfNwkJoinFailedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_FAILED_TO_JOIN: Status: %02x Rejoin:%02x\n",
        pEvent->u8Status,
        pEvent->bRejoin);
}

void vDumpNwkLeaveConfirm(ZPS_tsAfNwkLeaveConfEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_LEAVE_CONFIRM: PanID: %016llx Status: %02x Rejoin:%02x\n",
        pEvent->u64ExtAddr,
        pEvent->eStatus,
        pEvent->bRejoin);
}

void vDumpAfEvent(ZPS_tsAfEvent* psStackEvent)
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
            //vDumpDiscoveryCompleteEvent(&psStackEvent->uEvent.sNwkDiscoveryEvent);
            break;

        case ZPS_EVENT_NWK_LEAVE_CONFIRM:
            vDumpNwkLeaveConfirm(&psStackEvent->uEvent.sNwkLeaveConfirmEvent);
            break;

        default:
            DBG_vPrintf(TRUE, "Unknown Zigbee stack event: event type %d\n", psStackEvent->eType);
            break;
    }
}

void vDumpNetworkParameters()
{
    DBG_vPrintf(TRUE, "Current network parameters\n");
    DBG_vPrintf(TRUE, "    Device type: %d\n", ZPS_eAplZdoGetDeviceType());
    DBG_vPrintf(TRUE, "    PanID: 0x%04x\n", ZPS_u16AplZdoGetNetworkPanId());
    DBG_vPrintf(TRUE, "    Extended PanID: 0x%016llx\n", ZPS_u64AplZdoGetNetworkExtendedPanId());
    DBG_vPrintf(TRUE, "    Radio Channel: %d\n", ZPS_u8AplZdoGetRadioChannel());
    DBG_vPrintf(TRUE, "    Network addr: %04x\n", ZPS_u16AplZdoGetNwkAddr());
    DBG_vPrintf(TRUE, "    IEEE Addr: 0x%016llx\n\n", ZPS_u64AplZdoGetIeeeAddr());

    ZPS_tsAplAib * aib = ZPS_psAplAibGetAib();
    DBG_vPrintf(TRUE, "    u64ApsTrustCenterAddress: 0x%016llx\n", aib->u64ApsTrustCenterAddress);
    DBG_vPrintf(TRUE, "    u64ApsUseExtendedPanid: 0x%016llx\n", aib->u64ApsUseExtendedPanid);
    DBG_vPrintf(TRUE, "    bApsDesignatedCoordinator: %d\n", aib->bApsDesignatedCoordinator);
    DBG_vPrintf(TRUE, "    bApsUseInsecureJoin: %d\n", aib->bApsUseInsecureJoin);
    DBG_vPrintf(TRUE, "    bDecryptInstallCode: %d\n", aib->bDecryptInstallCode);
    DBG_vPrintf(TRUE, "    u8KeyType: %d\n\n", aib->u8KeyType);

    DBG_vPrintf(TRUE, "    sBDB.eState: %d\n", sBDB.eState);
    DBG_vPrintf(TRUE, "    sBDB.u8bdbCommissioningMode: %d\n", sBDB.sAttrib.u8bdbCommissioningMode);
    DBG_vPrintf(TRUE, "    sBDB.ebdbCommissioningStatus: %d\n", sBDB.sAttrib.ebdbCommissioningStatus);
    DBG_vPrintf(TRUE, "    sBDB.bbdbNodeIsOnANetwork: %d\n", sBDB.sAttrib.bbdbNodeIsOnANetwork);
    DBG_vPrintf(TRUE, "    sBDB.u64bdbJoiningNodeEui64: 0x%016llx\n", sBDB.sAttrib.u64bdbJoiningNodeEui64);
    DBG_vPrintf(TRUE, "    sBDB.u8bdbNodeJoinLinkKeyType: %d\n", sBDB.sAttrib.u8bdbNodeJoinLinkKeyType);
    DBG_vPrintf(TRUE, "    sBDB.bLeaveRequested: %d\n", sBDB.sAttrib.bLeaveRequested);
}

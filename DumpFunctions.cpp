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

const char * getClusterName(uint16 clusterID)
{
    switch(clusterID)
    {
        case ZPS_ZDP_DEVICE_ANNCE_REQ_CLUSTER_ID: return "Device announce request";
        case ZPS_ZDP_NODE_DESC_REQ_CLUSTER_ID: return "Node descriptor request";
        case ZPS_ZDP_SIMPLE_DESC_REQ_CLUSTER_ID: return "Simple descriptor request";
        case ZPS_ZDP_ACTIVE_EP_REQ_CLUSTER_ID: return "Active endpoint request";
        case ZPS_ZDP_BIND_REQ_CLUSTER_ID: return "Bind request";
        case ZPS_ZDP_UNBIND_REQ_CLUSTER_ID: return "Unbind request";
        case ZPS_ZDP_MGMT_LQI_REQ_CLUSTER_ID: return "Mgmt LQI request";
        case ZPS_ZDP_MGMT_RTG_REQ_CLUSTER_ID: return "Mgmt routing request";
        case ZPS_ZDP_MGMT_BIND_REQ_CLUSTER_ID: return "Mgmt bind request";

        case ZPS_ZDP_NODE_DESC_RSP_CLUSTER_ID: return "Node descriptor response";
        case ZPS_ZDP_SIMPLE_DESC_RSP_CLUSTER_ID: return "Simple descriptor response";
        case ZPS_ZDP_ACTIVE_EP_RSP_CLUSTER_ID: return "Active endpoint response";
        case ZPS_ZDP_MGMT_LQI_RSP_CLUSTER_ID: return "Mgmt LQI response";
        case ZPS_ZDP_MGMT_RTG_RSP_CLUSTER_ID: return "Mgmt routing response";
        case ZPS_ZDP_MGMT_BIND_RSP_CLUSTER_ID: return "Mgmt bind response";
    }

    return "???";
}

void vDumpDataIndicationEvent(ZPS_tsAfDataIndEvent * pEvent)
{
    const char * clusterName = "";
    if(pEvent->u8DstEndpoint == 0)
        clusterName = getClusterName(pEvent->u16ClusterId);

    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_INDICATION: SrcEP=%d DstEP=%d SrcAddr=%04x Cluster=%04x (%s) Status=%d\n",
            pEvent->u8SrcEndpoint,
            pEvent->u8DstEndpoint,
            pEvent->uSrcAddress.u16Addr,
            pEvent->u16ClusterId,
            clusterName,
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
    const char * clusterName = "";
    if(pEvent->u8SrcEndpoint == 0)
        clusterName = getClusterName(pEvent->u16ClusterId);

    DBG_vPrintf(TRUE, "ZPS_EVENT_APS_DATA_ACK: SrcEP=%d DrcEP=%d DstAddr=%04x Profile=%04x Cluster=%04x (%s)\n",
                pEvent->u8SrcEndpoint,
                pEvent->u8DstEndpoint,
                pEvent->u16DstAddr,
                pEvent->u16ProfileId,
                pEvent->u16ClusterId,
                clusterName);
}

void vDumpJoinedAsRouterEvent(ZPS_tsAfNwkJoinedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_JOINED_AS_ROUTER: Addr=%04x, rejoin=%d, secured rejoin=%d\n",
                pEvent->u16Addr,
                pEvent->bRejoin,
                pEvent->bSecuredRejoin);
}

void vDumpJoinedAsEndDeviceEvent(ZPS_tsAfNwkJoinedEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_JOINED_AS_END_DEVICE: Addr=%04x, rejoin=%d, secured rejoin=%d\n",
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

void vDumpNwkPollConfirm(ZPS_tsAfPollConfEvent * pEvent)
{
    if(pEvent->u8Status == MAC_ENUM_SUCCESS)
        DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_POLL_CONFIRM: status=Success\n");
    else if(pEvent->u8Status == MAC_ENUM_NO_ACK)
        DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_POLL_CONFIRM: status=No ACK\n");
    else if(pEvent->u8Status == MAC_ENUM_NO_DATA)
        DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_POLL_CONFIRM: status=No Data\n");
    else
        DBG_vPrintf(TRUE, "ZPS_EVENT_NWK_POLL_CONFIRM: status=%d\n",
        pEvent->u8Status);
}

void vDumpBindEvent(ZPS_tsAfZdoBindEvent * pEvent)
{
    if(pEvent->u8DstAddrMode == ZPS_E_ADDR_MODE_IEEE)
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_BIND: SrcEP=%d DstEP=%d DstAddr=%016llx\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->uDstAddr.u64Addr);
    else if(pEvent->u8DstAddrMode == ZPS_E_ADDR_MODE_SHORT)
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_BIND: SrcEP=%d DstEP=%d DstAddr=%04x\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->uDstAddr.u16Addr);
    else
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_BIND: SrcEP=%d DstEP=%d Unknown DstAddrMode=%d\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->u8DstAddrMode);
}

void vDumpUnbindEvent(ZPS_tsAfZdoUnbindEvent * pEvent)
{
    if(pEvent->u8DstAddrMode == ZPS_E_ADDR_MODE_IEEE)
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_UNBIND: SrcEP=%d DstEP=%d DstAddr=%016llx\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->uDstAddr.u64Addr);
    else if(pEvent->u8DstAddrMode == ZPS_E_ADDR_MODE_SHORT)
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_UNBIND: SrcEP=%d DstEP=%d DstAddr=%04x\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->uDstAddr.u16Addr);
    else
        DBG_vPrintf(TRUE, "ZPS_EVENT_ZDO_UNBIND: SrcEP=%d DstEP=%d Unknown DstAddrMode=%d\n",
            pEvent->u8SrcEp,
            pEvent->u8DstEp,
            pEvent->uDstAddr);
}

void vDumpTrustCenterStatusEvent(ZPS_tsAfTCstatusEvent * pEvent)
{
    DBG_vPrintf(TRUE, "ZPS_EVENT_TC_STATUS: status=0x%02x\n", pEvent->u8Status);
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

        case ZPS_EVENT_NWK_JOINED_AS_ENDDEVICE:
            vDumpJoinedAsEndDeviceEvent(&psStackEvent->uEvent.sNwkJoinedEvent);
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

        case ZPS_EVENT_NWK_POLL_CONFIRM:
            vDumpNwkPollConfirm(&psStackEvent->uEvent.sNwkPollConfirmEvent);
            break;

        case ZPS_EVENT_ZDO_BIND:
            vDumpBindEvent(&psStackEvent->uEvent.sZdoBindEvent);
            break;

        case ZPS_EVENT_ZDO_UNBIND:
            vDumpUnbindEvent(&psStackEvent->uEvent.sZdoBindEvent);
            break;

        case ZPS_EVENT_TC_STATUS:
            vDumpTrustCenterStatusEvent(&psStackEvent->uEvent.sApsTcEvent);
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

void vDisplayNeighbourTable( void )
{
    void * thisNet = ZPS_pvAplZdoGetNwkHandle();
    ZPS_tsNwkNib * thisNib = ZPS_psNwkNibGetHandle(thisNet);
    uint8 i;

    DBG_vPrintf(TRUE, "\n+++++++ Neighbour Table Size: %d\n", thisNib->sTblSize.u16NtActv);

    for( i = 0 ; i < thisNib->sTblSize.u16NtActv ; i++ )
    {

        DBG_vPrintf(TRUE, "    SAddr: 0x%04x - ExtAddr: 0x%016llx - LQI: %3i - Failed TX's: %i - Auth: %i - %i %i %i %i %i %i - Active: %i - %i %i %i\n",
                    thisNib->sTbl.psNtActv[i].u16NwkAddr,
                    ZPS_u64NwkNibGetMappedIeeeAddr(thisNet,thisNib->sTbl.psNtActv[i].u16Lookup),
                    thisNib->sTbl.psNtActv[i].u8LinkQuality,
                    thisNib->sTbl.psNtActv[i].u8TxFailed,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1Authenticated,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1DeviceType,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1ExpectAnnc,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1LinkStatusDone,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1PowerSource,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1RxOnWhenIdle,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1SecurityMode,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u1Used,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u2Relationship,
                    thisNib->sTbl.psNtActv[i].u8Age,
                    thisNib->sTbl.psNtActv[i].uAncAttrs.bfBitfields.u3OutgoingCost
                    );

    }
}

void vDisplayDiscoveredNodes(void)
{
    ZPS_tsNwkNib * thisNib;
    uint8 i;

    thisNib = ZPS_psNwkNibGetHandle(ZPS_pvAplZdoGetNwkHandle());

    DBG_vPrintf(TRUE, "\n++++++ Discovered nodes\n");
    for( i = 0; i < thisNib->sTblSize.u8NtDisc; i++)
    {
        DBG_vPrintf(TRUE, "  Index: %d", i );

        DBG_vPrintf(TRUE, "    EPID: %016llx", thisNib->sTbl.psNtDisc[i].u64ExtPanId);

        DBG_vPrintf(TRUE, "    PAN: %04x", thisNib->sTbl.psNtDisc[i].u16PanId);

        DBG_vPrintf(TRUE, "    SAddr: %04x", thisNib->sTbl.psNtDisc[i].u16NwkAddr);

        DBG_vPrintf(TRUE, "    LQI %d\n", thisNib->sTbl.psNtDisc[i].u8LinkQuality);

        DBG_vPrintf(TRUE, "    CH: %d", thisNib->sTbl.psNtDisc[i].u8LogicalChan);

        DBG_vPrintf(TRUE, "    PJ: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u1JoinPermit);

        DBG_vPrintf(TRUE, "    Coord: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u1PanCoord);

        DBG_vPrintf(TRUE, "    RT Cap: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u1ZrCapacity);

        DBG_vPrintf(TRUE, "    ED Cap: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u1ZedCapacity);

        DBG_vPrintf(TRUE, "    Depth: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u4Depth);

        DBG_vPrintf(TRUE, "    StPro: %d", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u4StackProfile);

        DBG_vPrintf(TRUE, "    PP: %d\r\n", thisNib->sTbl.psNtDisc[i].uAncAttrs.bfBitfields.u1PotentialParent);
    }
}
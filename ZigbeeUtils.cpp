#include "ZigbeeUtils.h"

extern "C"
{
    #include "dbg.h"
    #include "zcl.h"
}

bool hasBindings(uint8 ep, uint16 clusterID)
{
    // Get pointers
    ZPS_tsAplAib * aib = ZPS_psAplAibGetAib();
    ZPS_tsAplApsmeBindingTableType * bindingTable = aib->psAplApsmeAibBindingTable;
    ZPS_tsAplApsmeBindingTable* table = bindingTable->psAplApsmeBindingTable;

    if(!table)
        return false;

    for(uint32 i=0; i < table->u32SizeOfBindingTable; i++)
    {
        ZPS_tsAplApsmeBindingTableStoreEntry * entry = table->pvAplApsmeBindingTableEntryForSpSrcAddr + i;
        if(entry->u8SourceEndpoint == ep && entry->u16ClusterId == clusterID)
            return true;
    }

    return false;
}

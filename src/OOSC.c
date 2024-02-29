// This file originated from NXP Zigbee SDK, created by Leo Mitchell in 2012
// Note that functions and definitions were significantly extended since originally developed

#include <jendefs.h>
#include "zcl.h"
#include "OOSC.h"
#include "zcl_options.h"
#include "OOSC.h"

#ifdef CLD_OOSC

const tsZCL_AttributeDefinition asCLD_OOSCClusterAttributeDefinitions[] = {
#ifdef OOSC_SERVER
    {E_CLD_OOSC_ATTR_ID_SWITCH_TYPE,            E_ZCL_AF_RD,                            E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eSwitchMode), 0},    // Mandatory
    {E_CLD_OOSC_ATTR_ID_SWITCH_ACTIONS,         (E_ZCL_AF_RD|E_ZCL_AF_WR),              E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eSwitchActions), 0}, // Mandatory

    // Custom attributes
    {E_CLD_OOSC_ATTR_ID_SWITCH_MODE,            (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eSwitchMode), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_RELAY_MODE,      (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eRelayMode), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_MAX_PAUSE,       (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_UINT16,   (uint32)(&((tsCLD_OOSC*)(0))->iMaxPause), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_DUR,  (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_UINT16,   (uint32)(&((tsCLD_OOSC*)(0))->iMinLongPress), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_MODE, (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eLongPressMode), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_OPERATION_MODE,  (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eOperationMode), 0},
    {E_CLD_OOSC_ATTR_ID_SWITCH_INTERLOCK_MODE,  (E_ZCL_AF_RD|E_ZCL_AF_WR|E_ZCL_AF_MS),  E_ZCL_ENUM8,    (uint32)(&((tsCLD_OOSC*)(0))->eInterlockMode), 0},

#endif        
    {E_CLD_GLOBAL_ATTR_ID_CLUSTER_REVISION,     (E_ZCL_AF_RD|E_ZCL_AF_GA),              E_ZCL_UINT16,   (uint32)(&((tsCLD_OOSC*)(0))->u16ClusterRevision), 0},   // Mandatory

};

tsZCL_ClusterDefinition sCLD_OOSC = {
        GENERAL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION,
        FALSE,
        E_ZCL_SECURITY_NETWORK,
        (sizeof(asCLD_OOSCClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition)),
        (tsZCL_AttributeDefinition*)asCLD_OOSCClusterAttributeDefinitions,
        NULL
};

uint8 au8OOSCAttributeControlBits[(sizeof(asCLD_OOSCClusterAttributeDefinitions) / sizeof(tsZCL_AttributeDefinition))];

PUBLIC  teZCL_Status eCLD_OOSCCreateOnOffSwitchConfig(
                tsZCL_ClusterInstance              *psClusterInstance,
                bool_t                              bIsServer,
                tsZCL_ClusterDefinition            *psClusterDefinition,
                void                               *pvEndPointSharedStructPtr,
                uint8                              *pu8AttributeControlBits)
{

    #ifdef STRICT_PARAM_CHECK 
        /* Parameter check */
        if(psClusterInstance==NULL)
        {
            return E_ZCL_ERR_PARAMETER_NULL;
        }
    #endif

    // cluster data
    vZCL_InitializeClusterInstance(
                                   psClusterInstance, 
                                   bIsServer,
                                   psClusterDefinition,
                                   pvEndPointSharedStructPtr,
                                   pu8AttributeControlBits,
                                   NULL,
                                   NULL);    

        if(pvEndPointSharedStructPtr != NULL)
        {
#ifdef OOSC_SERVER            
            /* Set attribute defaults */
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eSwitchMode = E_CLD_OOSC_TYPE_TOGGLE;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eSwitchActions = E_CLD_OOSC_ACTION_TOGGLE;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eRelayMode = RELAY_MODE_FRONT;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->iMaxPause = 250;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->iMinLongPress = 1000;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eLongPressMode = E_CLD_OOSC_LONG_PRESS_MODE_NONE;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eOperationMode = E_CLD_OOSC_OPERATION_MODE_SERVER;
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->eOperationMode = E_CLD_OOSC_INTERLOCK_MODE_NONE;
#endif
            ((tsCLD_OOSC*)psClusterInstance->pvEndPointSharedStructPtr)->u16ClusterRevision = CLD_OOSC_CLUSTER_REVISION;
        }

    return E_ZCL_SUCCESS;

}

#endif

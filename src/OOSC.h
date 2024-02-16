// This file originated from NXP Zigbee SDK, created by Leo Mitchell in 2012
// Note that functions and definitions were significantly extended since originally developed

#ifndef OOSC_H
#define OOSC_H

#include "ButtonModes.h"

#include <jendefs.h>
#include "zcl.h"
#include "zcl_options.h"

// Cluster ID's
#define GENERAL_CLUSTER_ID_ONOFF_SWITCH_CONFIGURATION   0x0007

#ifndef CLD_OOSC_CLUSTER_REVISION
    #define CLD_OOSC_CLUSTER_REVISION                         1
#endif 

// On/Off switch configuration attribute ID's (3.9.2.2)
typedef enum 
{
    E_CLD_OOSC_ATTR_ID_SWITCH_TYPE              = 0x0000,   /* Mandatory */
    E_CLD_OOSC_ATTR_ID_SWITCH_ACTIONS           = 0x0010,   /* Mandatory */

    // Custom attributes
    E_CLD_OOSC_ATTR_ID_SWITCH_MODE              = 0xff00,
    E_CLD_OOSC_ATTR_ID_SWITCH_RELAY_MODE        = 0xff01,
    E_CLD_OOSC_ATTR_ID_SWITCH_MAX_PAUSE         = 0xff02,
    E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_DUR    = 0xff03,
    E_CLD_OOSC_ATTR_ID_SWITCH_LONG_PRESS_MODE   = 0xff04,
    E_CLD_OOSC_ATTR_ID_SWITCH_OPERATION_MODE    = 0xff05,
    E_CLD_OOSC_ATTR_ID_SWITCH_INTERLOCK_MODE    = 0xff06,
} teCLD_OOSC_ClusterID;


// On/Off switch types (modes)
typedef enum 
{
    E_CLD_OOSC_TYPE_TOGGLE,
    E_CLD_OOSC_TYPE_MOMENTARY,
    E_CLD_OOSC_TYPE_MULTI_FUNCTION
} teCLD_OOSC_SwitchType;

// On/Off switch actions
typedef enum 
{
    E_CLD_OOSC_ACTION_S2ON_S1OFF,
    E_CLD_OOSC_ACTION_S2OFF_S1ON,
    E_CLD_OOSC_ACTION_TOGGLE
} teCLD_OOSC_SwitchAction;

// Long Press modes
typedef enum
{
    E_CLD_OOSC_LONG_PRESS_MODE_NONE,
    E_CLD_OOSC_LONG_PRESS_MODE_LEVEL_CTRL_UP,
    E_CLD_OOSC_LONG_PRESS_MODE_LEVEL_CTRL_DOWN
} teCLD_OOSC_LongPressMode;

// Operation mode
typedef enum 
{
    E_CLD_OOSC_OPERATION_MODE_SERVER,
    E_CLD_OOSC_OPERATION_MODE_CLIENT
} teCLD_OOSC_OperationMode;

// Interlock mode
typedef enum 
{
    E_CLD_OOSC_INTERLOCK_MODE_NONE,
    E_CLD_OOSC_INTERLOCK_MODE_MUTEX,
    E_CLD_OOSC_INTERLOCK_MODE_OPPOSITE
} teCLD_OOSC_InterlockMode;


// On/Off Switch Configuration Cluster
typedef struct
{
#ifdef OOSC_SERVER    
    zenum8                  eSwitchMode;                /* Mandatory */
    zenum8                  eSwitchActions;             /* Mandatory */

    // Custom attrs
    zenum8                  eRelayMode;
    zuint16                 iMaxPause;
    zuint16                 iMinLongPress;
    zenum8                  eLongPressMode;
    zenum8                  eOperationMode;
    zenum8                  eInterlockMode;

#endif    
    zuint16                 u16ClusterRevision;
} tsCLD_OOSC;


PUBLIC teZCL_Status eCLD_OOSCCreateOnOffSwitchConfig(
                tsZCL_ClusterInstance              *psClusterInstance,
                bool_t                              bIsServer,
                tsZCL_ClusterDefinition            *psClusterDefinition,
                void                               *pvEndPointSharedStructPtr,
                uint8              *pu8AttributeControlBits);


extern tsZCL_ClusterDefinition sCLD_OOSC;
extern uint8 au8OOSCAttributeControlBits[];
extern const tsZCL_AttributeDefinition asCLD_OOSCClusterAttributeDefinitions[];

#endif /* OOSC_H */

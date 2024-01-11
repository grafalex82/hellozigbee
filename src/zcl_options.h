#ifndef ZCL_OPTIONS_H
#define ZCL_OPTIONS_H

#include <jendefs.h>

// General device options
//////////////////////////////

// Use NXP manufacturer code as we do not have our own
#define ZCL_MANUFACTURER_CODE                                0x1037

/* Sets the number of endpoints that will be created by the ZCL library */
#define ZCL_NUMBER_OF_ENDPOINTS                             3

#define ZCL_ATTRIBUTE_READ_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_READ_CLIENT_SUPPORTED
#define ZCL_ATTRIBUTE_WRITE_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_REPORTING_SERVER_SUPPORTED
#define ZCL_CONFIGURE_ATTRIBUTE_REPORTING_SERVER_SUPPORTED


#define ZCL_NUMBER_OF_REPORTS     10
#define ZLO_MIN_REPORT_INTERVAL   1
#define ZLO_MAX_REPORT_INTERVAL   15

/* Enable wild card profile */
#define ZCL_ALLOW_WILD_CARD_PROFILE


// Various cluster definitions
//////////////////////////////

#define CLD_BASIC
#define BASIC_SERVER
#define BASIC_CLIENT

#define CLD_GROUPS
#define GROUPS_SERVER

//#define CLD_SCENES
//#define SCENES_CLIENT

#define CLD_IDENTIFY
#define IDENTIFY_SERVER
#define CLD_IDENTIFY_CMD_TRIGGER_EFFECT

#define CLD_ONOFF
#define ONOFF_CLIENT
#define ONOFF_SERVER

#define CLD_OOSC
#define OOSC_SERVER

#define CLD_LEVEL_CONTROL
#define LEVEL_CONTROL_CLIENT

#define CLD_BIND_SERVER
#define MAX_NUM_BIND_QUEUE_BUFFERS 4
#define MAX_PDU_BIND_QUEUE_PAYLOAD_SIZE 100

#define CLD_MULTISTATE_INPUT_BASIC
#define MULTISTATE_INPUT_BASIC_SERVER
#define CLD_MULTISTATE_INPUT_BASIC_ATTR_NUMBER_OF_STATES 255

#define CLD_OTA
#define OTA_CLIENT
#define OTA_NO_CERTIFICATE
#define OTA_CLD_ATTR_FILE_OFFSET
#define OTA_CLD_ATTR_CURRENT_FILE_VERSION
#define OTA_CLD_ATTR_CURRENT_ZIGBEE_STACK_VERSION
#define OTA_MAX_BLOCK_SIZE 48
#define OTA_TIME_INTERVAL_BETWEEN_RETRIES 10
#define OTA_STRING_COMPARE
#define OTA_UPGRADE_VOLTAGE_CHECK


// Basic cluster settings
//////////////////////////////

#define   CLD_BAS_ATTR_APPLICATION_VERSION
#define   CLD_BAS_ATTR_STACK_VERSION
#define   CLD_BAS_ATTR_HARDWARE_VERSION
#define   CLD_BAS_ATTR_MANUFACTURER_NAME
#define   CLD_BAS_ATTR_MODEL_IDENTIFIER
#define   CLD_BAS_ATTR_DATE_CODE
#define   CLD_BAS_ATTR_SW_BUILD_ID
#define   CLD_BAS_ATTR_GENERIC_DEVICE_CLASS
#define   CLD_BAS_ATTR_GENERIC_DEVICE_TYPE


#define CLD_BAS_APP_VERSION         (1)
#define CLD_BAS_STACK_VERSION       (2)
#define CLD_BAS_HARDWARE_VERSION    (1)
#define CLD_BAS_MANUF_NAME_STR      "NXP"
#define CLD_BAS_MANUF_NAME_SIZE     3
#define CLD_BAS_MODEL_ID_STR        "Hello Zigbee Switch"
#define CLD_BAS_MODEL_ID_SIZE       19
#define CLD_BAS_DATE_STR            "20210331"
#define CLD_BAS_DATE_SIZE           8
#define CLD_BAS_POWER_SOURCE        E_CLD_BAS_PS_BATTERY
#define CLD_BAS_SW_BUILD_STR        "v0.1"
#define CLD_BAS_SW_BUILD_SIZE       4
#define CLD_BAS_DEVICE_CLASS        (0)


#endif /* ZCL_OPTIONS_H */

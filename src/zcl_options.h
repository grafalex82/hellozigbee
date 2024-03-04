#ifndef ZCL_OPTIONS_H
#define ZCL_OPTIONS_H

#include <jendefs.h>

// General device options
//////////////////////////////

// Use NXP manufacturer code as we do not have our own
#define ZCL_MANUFACTURER_CODE               0x1037

#define ZCL_ATTRIBUTE_READ_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_READ_CLIENT_SUPPORTED
#define ZCL_ATTRIBUTE_WRITE_SERVER_SUPPORTED
#define ZCL_ATTRIBUTE_REPORTING_SERVER_SUPPORTED
#define ZCL_CONFIGURE_ATTRIBUTE_REPORTING_SERVER_SUPPORTED


#define ZCL_NUMBER_OF_REPORTS               10
#define ZCL_SYSTEM_MAX_REPORT_INTERVAL      15

// Enable wild card profile
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
#define MAX_NUM_BIND_QUEUE_BUFFERS                          4
#define MAX_PDU_BIND_QUEUE_PAYLOAD_SIZE                     100

#define CLD_MULTISTATE_INPUT_BASIC
#define MULTISTATE_INPUT_BASIC_SERVER
#define CLD_MULTISTATE_INPUT_BASIC_ATTR_NUMBER_OF_STATES    255

#define CLD_DEVICE_TEMPERATURE_CONFIGURATION
#define DEVICE_TEMPERATURE_CONFIGURATION_SERVER

#define CLD_OTA
#define OTA_CLIENT
#define OTA_NO_CERTIFICATE
#define OTA_CLD_ATTR_FILE_OFFSET
#define OTA_CLD_ATTR_CURRENT_FILE_VERSION
#define OTA_CLD_ATTR_CURRENT_ZIGBEE_STACK_VERSION
#define OTA_MAX_BLOCK_SIZE                                  48
#define OTA_TIME_INTERVAL_BETWEEN_RETRIES                   10
#define OTA_STRING_COMPARE
#define OTA_UPGRADE_VOLTAGE_CHECK


// Basic cluster settings
//////////////////////////////

#define CLD_BAS_ATTR_APPLICATION_VERSION
#define CLD_BAS_ATTR_STACK_VERSION
#define CLD_BAS_ATTR_HARDWARE_VERSION
#define CLD_BAS_ATTR_MANUFACTURER_NAME
#define CLD_BAS_ATTR_MODEL_IDENTIFIER
#define CLD_BAS_ATTR_DATE_CODE
#define CLD_BAS_ATTR_SW_BUILD_ID
#define CLD_BAS_ATTR_GENERIC_DEVICE_CLASS
#define CLD_BAS_ATTR_GENERIC_DEVICE_TYPE


#define CLD_BAS_APP_VERSION                                 (1)
#define CLD_BAS_STACK_VERSION                               (2)
#define CLD_BAS_HARDWARE_VERSION                            (1)
#define CLD_BAS_MANUF_NAME_STR                              "DIY"
#define CLD_BAS_MANUF_NAME_SIZE                             3
#define CLD_BAS_DATE_STR                                    "20210331"
#define CLD_BAS_DATE_SIZE                                   8
#define CLD_BAS_POWER_SOURCE                                E_CLD_BAS_PS_BATTERY
#define CLD_BAS_SW_BUILD_STR                                "v0.1"
#define CLD_BAS_SW_BUILD_SIZE                               4
#define CLD_BAS_DEVICE_CLASS                                (0)

#define ALT_PIN_TIMER                                       (0x80)

#if defined(TARGET_BOARD_EBYTE_E75)     // This section describe development board based on EBYTE E75–2G4M10S module
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.E75-2G4M10S"
    #define CLD_BAS_MODEL_ID_SIZE       24

    #define HEARTBEAT_LED_PIN           (4)

    #define NUM_BUTTONS                 (2)
    #define HAS_VIRTUAL_CHANNELS        

    #define SWITCH1_BTN_BIT             (1)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT             (2)
    #define SWITCH2_BTN_MASK            (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER              (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER             (E_AHI_TIMER_4)
    #define LED2_RED_TIMER              (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER             (E_AHI_TIMER_2)

    #define BASIC_ENDPOINT              (EBYTE_E75_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (EBYTE_E75_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT            (EBYTE_E75_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT            (EBYTE_E75_SWITCHB_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (4)

#elif defined(TARGET_BOARD_QBKG11LM)    // This section describes QBKG11LM Xiaomi Aqara 1-gang switch
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.QBKG11LM"
    #define CLD_BAS_MODEL_ID_SIZE       21

    // TODO: Fill proper pin numbers

    #define HEARTBEAT_LED_PIN           (19)

    #define NUM_BUTTONS                 (1)

    #define SWITCH1_BTN_BIT             (10)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_BIT)

    #define LED1_RED_TIMER              (E_AHI_TIMER_1)
    #define LED1_BLUE_TIMER             (E_AHI_TIMER_0 | ALT_PIN_TIMER)

    #define BASIC_ENDPOINT              (QBKG11LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (QBKG11LM_SWITCH1_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (2)

#elif defined(TARGET_BOARD_QBKG12LM)    // This section describes QBKG12LM Xiaomi Aqara 2-gang switch
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.QBKG12LM"
    #define CLD_BAS_MODEL_ID_SIZE       21

    // TODO: Fill proper pin numbers

    #define NUM_BUTTONS                 (2)
    #define HAS_VIRTUAL_CHANNELS

    #define HEARTBEAT_LED_PIN           (19)

    #define SWITCH1_BTN_BIT             (1)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT             (2)
    #define SWITCH2_BTN_MASK            (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER              (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER             (E_AHI_TIMER_4)
    #define LED2_RED_TIMER              (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER             (E_AHI_TIMER_2)

    #define BASIC_ENDPOINT              (QBKG12LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (QBKG12LM_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT            (QBKG12LM_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT            (QBKG12LM_SWITCHB_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (4)

#endif // TARGET_BOARD


#endif /* ZCL_OPTIONS_H */

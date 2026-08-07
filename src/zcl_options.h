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
#define CLD_BAS_DATE_STR                                    BUILD_DATE
#define CLD_BAS_DATE_SIZE                                   BUILD_DATE_LEN
// All current target boards (E75, QBKG11LM, QBKG12LM) are mains-powered Zigbee routers
// (LogicalType="ZR", RxOnWhenIdle="true" in HelloZigbee.zpscfg). Report mains here so the Basic
// cluster power-source attribute matches the node descriptor; otherwise coordinators (e.g. z2m)
// treat the device as a sleepy battery node and defer OTA/availability handling. A future
// battery/end-device board should override this in its own TARGET_BOARD_* section below.
#define CLD_BAS_POWER_SOURCE                                E_CLD_BAS_PS_SINGLE_PHASE_MAINS
#define CLD_BAS_SW_BUILD_STR                                VERSION_STR
#define CLD_BAS_SW_BUILD_SIZE                               VERSION_STR_LEN
#define CLD_BAS_DEVICE_CLASS                                (0)

#define ALT_PIN_TIMER                                       (0x80)

#if defined(TARGET_BOARD_EBYTE_E75)     // This section describe development board based on EBYTE E75–2G4M10S module
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.E75-2G4M10S"
    #define CLD_BAS_MODEL_ID_SIZE       24

    #define HEARTBEAT_LED_PIN           (4)
    #define HEARTBEAT_LED_MASK          (1UL << HEARTBEAT_LED_PIN)

    #define SWITCH1_BTN_PIN             (1)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_PIN)
    #define SWITCH2_BTN_PIN             (2)
    #define SWITCH2_BTN_MASK            (1UL << SWITCH2_BTN_PIN)

    #define SUPPORTS_PWM_LED
    #define LED1_RED_MASK_OR_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_MASK_OR_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_MASK_OR_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_MASK_OR_TIMER     (E_AHI_TIMER_2)

    #define BASIC_ENDPOINT              (EBYTE_E75_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (EBYTE_E75_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT            (EBYTE_E75_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT            (EBYTE_E75_SWITCHB_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (4)

#elif defined(TARGET_BOARD_QBKG11LM)    // This section describes QBKG11LM Xiaomi Aqara 1-gang switch
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.QBKG11LM"
    #define CLD_BAS_MODEL_ID_SIZE       21

    //#define HEARTBEAT_LED_PIN           (X)  // No spare pins for the heartbeat LED
    //#define HEARTBEAT_LED_MASK          (1UL << HEARTBEAT_LED_PIN)

    #define SWITCH1_BTN_PIN             (10)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_PIN)

    //#define SUPPORTS_PWM_LED          // LEDs on the QBKG11LM are connected to non-PWM pins
    #define LED1_RED_PIN_1              (19)    // The QBKG11LM has 2 pairs of LEDs, but they are used synchronously
    #define LED1_RED_PIN_2              (11)
    #define LED1_RED_MASK_OR_TIMER      (1UL << LED1_RED_PIN_1) | (1UL << LED1_RED_PIN_2)
    #define LED1_BLUE_PIN_1             (5)
    #define LED1_BLUE_PIN_2             (4)
    #define LED1_BLUE_MASK_OR_TIMER     (1UL << LED1_BLUE_PIN_1) | (1UL << LED1_BLUE_PIN_2)

    #define RELAY1_ON_PIN               (13)
    #define RELAY1_ON_MASK              (1UL << RELAY1_ON_PIN)
    #define RELAY1_OFF_PIN              (12)
    #define RELAY1_OFF_MASK             (1UL << RELAY1_OFF_PIN)

    // On-board HLW8012 energy metering IC. CF (power) -> DIO8/PC1 and CF1
    // (voltage/current, SEL-muxed) -> DIO1/PC0 are the pulse counters' fixed
    // default inputs and need no pin configuration; SEL is inverted by a
    // 2N7002 on its way to the chip.
    #define SUPPORTS_POWER_METERING
    #define METERING_SEL_PIN            (9)
    #define METERING_SEL_MASK           (1UL << METERING_SEL_PIN)

    #define BASIC_ENDPOINT              (QBKG11LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (QBKG11LM_SWITCH1_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (2)

#elif defined(TARGET_BOARD_QBKG12LM)    // This section describes QBKG12LM Xiaomi Aqara 2-gang switch
    #define CLD_BAS_MODEL_ID_STR        "hello.zigbee.QBKG12LM"
    #define CLD_BAS_MODEL_ID_SIZE       21

    //#define HEARTBEAT_LED_PIN           (X)  // No spare pins for the heartbeat LED
    //#define HEARTBEAT_LED_MASK          (1UL << HEARTBEAT_LED_PIN)

    #define SWITCH1_BTN_PIN             (18)
    #define SWITCH1_BTN_MASK            (1UL << SWITCH1_BTN_PIN)
    #define SWITCH2_BTN_PIN             (10)
    #define SWITCH2_BTN_MASK            (1UL << SWITCH2_BTN_PIN)

    //#define SUPPORTS_PWM_LED          // LEDs on the QBKG12LM are connected to non-PWM pins
    #define LED1_RED_PIN                (19)
    #define LED1_RED_MASK_OR_TIMER      (1UL << LED1_RED_PIN)
    #define LED1_BLUE_PIN               (5)
    #define LED1_BLUE_MASK_OR_TIMER     (1UL << LED1_BLUE_PIN)
    #define LED2_RED_PIN                (11)
    #define LED2_RED_MASK_OR_TIMER      (1UL << LED2_RED_PIN)
    #define LED2_BLUE_PIN               (4)
    #define LED2_BLUE_MASK_OR_TIMER     (1UL << LED2_BLUE_PIN)

    #define RELAY1_ON_PIN               (17)
    #define RELAY1_ON_MASK              (1UL << RELAY1_ON_PIN)
    #define RELAY1_OFF_PIN              (16)
    #define RELAY1_OFF_MASK             (1UL << RELAY1_OFF_PIN)
    #define RELAY2_ON_PIN               (13)
    #define RELAY2_ON_MASK              (1UL << RELAY2_ON_PIN)
    #define RELAY2_OFF_PIN              (12)
    #define RELAY2_OFF_MASK             (1UL << RELAY2_OFF_PIN)

    // On-board HLW8012 energy metering IC. CF (power) -> DIO8/PC1 and CF1
    // (voltage/current, SEL-muxed) -> DIO1/PC0 are the pulse counters' fixed
    // default inputs and need no pin configuration; SEL is inverted by a
    // 2N7002 on its way to the chip.
    #define SUPPORTS_POWER_METERING
    #define METERING_SEL_PIN            (9)
    #define METERING_SEL_MASK           (1UL << METERING_SEL_PIN)

    #define BASIC_ENDPOINT              (QBKG12LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT            (QBKG12LM_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT            (QBKG12LM_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT            (QBKG12LM_SWITCHB_ENDPOINT)
    #define ZCL_NUMBER_OF_ENDPOINTS     (4)

#endif // TARGET_BOARD


// Electrical Measurement cluster is registered only on boards with a metering IC
#ifdef SUPPORTS_POWER_METERING
#define CLD_ELECTRICAL_MEASUREMENT
#define ELECTRICAL_MEASUREMENT_SERVER
#define CLD_ELECTMEAS_ATTR_ACTIVE_POWER
#define CLD_ELECTMEAS_ATTR_RMS_VOLTAGE
#define CLD_ELECTMEAS_ATTR_RMS_CURRENT
// Cumulative CF/CF1 pulse counts ride in two manufacturer-specific uint32
// attributes (0xFF00/0xFF01, manufacturer code 0x1037) so calibration can
// integrate over minutes instead of trusting one 1 s window
#define CLD_ELECTMEAS_ATTR_MAN_SPEC_APPARENT_POWER
#define CLD_ELECTMEAS_ATTR_MAN_SPEC_NON_ACTIVE_POWER
// Scaling attributes so coordinators can render real units
#define CLD_ELECTMEAS_ATTR_AC_POWER_MULTIPLIER
#define CLD_ELECTMEAS_ATTR_AC_POWER_DIVISOR
#define CLD_ELECTMEAS_ATTR_AC_VOLTAGE_MULTIPLIER
#define CLD_ELECTMEAS_ATTR_AC_VOLTAGE_DIVISOR
#define CLD_ELECTMEAS_ATTR_AC_CURRENT_MULTIPLIER
#define CLD_ELECTMEAS_ATTR_AC_CURRENT_DIVISOR
// Cumulative energy over the Simple Metering cluster (the SDK gates the
// cluster source on CLD_SIMPLE_METERING and the header structs on SM_SERVER)
#define CLD_SIMPLE_METERING
#define SM_SERVER
#define CLD_SM_ATTR_MULTIPLIER
#define CLD_SM_ATTR_DIVISOR
#endif

#endif /* ZCL_OPTIONS_H */

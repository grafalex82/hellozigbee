#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define ALT_PIN_TIMER           (0x80)

// TODO: This must come from CMake configuration
//#define TARGET_BOARD_EBYTE_E75 
//#define TARGET_BOARD_QBKG11LM


#if defined(TARGET_BOARD_EBYTE_E75)     // This section describe development board based on EBYTE E75–2G4M10S module
    #define TARGET_BOARD_NAME   "EBYTE E75-2G4M10S"

    #define HEARTBEAT_LED_PIN   (4)

    #define NUM_BUTTONS         (2)
    #define HAS_VIRTUAL_CHANNELS

    #define SWITCH1_BTN_BIT     (1)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT     (2)
    #define SWITCH2_BTN_MASK    (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER     (E_AHI_TIMER_2)

    #define BASIC_ENDPOINT      (EBYTE_E75_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT    (EBYTE_E75_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT    (EBYTE_E75_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT    (EBYTE_E75_SWITCHB_ENDPOINT)
    #define NUMBER_OF_ENDPOINTS (4)

#elif defined(TARGET_BOARD_QBKG11LM)    // This section describes QBKG11LM Xiaomi Aqara 1-gang switch
    #define TARGET_BOARD_NAME   "Xiaomi Aqara 1-gang switch"

    // TODO: Fill proper pin numbers

    #define HEARTBEAT_LED_PIN   (19)

    #define NUM_BUTTONS         (1)

    #define SWITCH1_BTN_BIT     (10)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_1)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_0 | ALT_PIN_TIMER)

    #define BASIC_ENDPOINT      (QBKG11LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT    (QBKG11LM_SWITCH1_ENDPOINT)
    #define NUMBER_OF_ENDPOINTS (2)

#elif defined(TARGET_BOARD_QBKG12LM)    // This section describes QBKG12LM Xiaomi Aqara 2-gang switch
    #define TARGET_BOARD_NAME   "Xiaomi Aqara 2-gang switch"

    // TODO: Fill proper pin numbers

    #define NUM_BUTTONS         (2)
    #define HAS_VIRTUAL_CHANNELS

    #define HEARTBEAT_LED_PIN   (19)

    #define SWITCH1_BTN_BIT     (1)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT     (2)
    #define SWITCH2_BTN_MASK    (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER     (E_AHI_TIMER_2)

    #define BASIC_ENDPOINT      (QBKG12LM_BASIC_ENDPOINT)
    #define SWITCH1_ENDPOINT    (QBKG12LM_SWITCH1_ENDPOINT)
    #define SWITCH2_ENDPOINT    (QBKG12LM_SWITCH2_ENDPOINT)
    #define SWITCHB_ENDPOINT    (QBKG12LM_SWITCHB_ENDPOINT)
    #define NUMBER_OF_ENDPOINTS (4)

#endif // TARGET_BOARD

#endif // CONFIGURATION_H

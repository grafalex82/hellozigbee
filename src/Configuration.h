#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// TODO: This must come from CMake configuration
#define TARGET_BOARD_EBYTE_E75 


#ifdef TARGET_BOARD_EBYTE_E75
    // This section describe development board based on EBYTE E75–2G4M10S module

    #define HEARTBEAT_LED_PIN   (4)

    #define NUM_BUTTONS         (2)
    #define HAS_VIRTUAL_CHANNELS

    #define SWITCH1_LED_PIN     (17)
    #define SWITCH2_LED_PIN     (12)

    #define SWITCH1_BTN_BIT     (1)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT     (2)
    #define SWITCH2_BTN_MASK    (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER     (E_AHI_TIMER_2)

#elif TARGET_BOARD_QBKG11LM
    // This section describes QBKG11LM Xiaomi Aqara 1-gang switch
    // TODO: Fill proper pin numbers

    #define HEARTBEAT_LED_PIN   (4)

    #define NUM_BUTTONS         (2)
    #define HAS_VIRTUAL_CHANNELS

    #define SWITCH1_LED_PIN     (17)
    #define SWITCH2_LED_PIN     (12)

    #define SWITCH1_BTN_BIT     (1)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT     (2)
    #define SWITCH2_BTN_MASK    (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER     (E_AHI_TIMER_2)

#elif TARGET_BOARD_QBKG12LM
    // This section describes QBKG12LM Xiaomi Aqara 2-gang switch
    // TODO: Fill proper pin numbers

    #define HEARTBEAT_LED_PIN   (4)

    #define NUM_BUTTONS         (2)
    #define HAS_VIRTUAL_CHANNELS

    #define SWITCH1_LED_PIN     (17)
    #define SWITCH2_LED_PIN     (12)

    #define SWITCH1_BTN_BIT     (1)
    #define SWITCH1_BTN_MASK    (1UL << SWITCH1_BTN_BIT)
    #define SWITCH2_BTN_BIT     (2)
    #define SWITCH2_BTN_MASK    (1UL << SWITCH2_BTN_BIT)

    #define LED1_RED_TIMER      (E_AHI_TIMER_3)
    #define LED1_BLUE_TIMER     (E_AHI_TIMER_4)
    #define LED2_RED_TIMER      (E_AHI_TIMER_1)
    #define LED2_BLUE_TIMER     (E_AHI_TIMER_2)

#endif // TARGET_BOARD

#endif // CONFIGURATION_H

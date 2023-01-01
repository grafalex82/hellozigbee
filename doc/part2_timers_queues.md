# Hello Zigbee World, Part 2 - Software timers and message queues

This is a second article in the series of learning NXP JN5169 microcontroller and Zigbee stack. In the [previous article](part1_bring_up.md) it was described bringing up a JN5169 microcontroller, and setting up some of the essential peripherals. As mentioned in [the very first article](part0_plan.md), the ultimate goal is to develop an alternative firmware for some Xiaomi Aqara Zigbee devices. But before I dive into Zigbee, I need to get an understanding of some building blocks provided by NXP SDK - software timers, message queues, and GPIO interrupts.

I'll be using the same cheap EBYTE E75-2G4M10S module accompanied with a few buttons and LED. Setting up a toolchain was also described in the previous article.

## Hello software timers

Software timers are a part of the Zigbee SDK provided by NXP, and described in [JN-UG-3113 ZigBee 3.0 Stack User Guide](https://www.nxp.com/docs/en/user-guide/JN-UG-3113.pdf). It can be used for scheduling relatively infrequent (milliseconds to seconds) events, measuring time intervals, as well as executing periodic tasks. Basically software timers component leverages a single hardware timer (tick timer), and use it to schedule multiple events on different timers in the application.

Let's try re-implementing the blinker using the software timers.

```cpp
ZTIMER_tsTimer timers[1];
uint8 blinkTimerHandle;

PUBLIC void vAppMain(void)
{
  // Initialize UART
  DBG_vUartInit(DBG_E_UART_0, DBG_E_UART_BAUD_RATE_115200);

  // Initialize hardware
  vAHI_DioSetDirection(0, BOARD_LED_CTRL_MASK);

  // Init and start timers
  ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));
  ZTIMER_eOpen(&blinkTimerHandle, blinkFunc, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
  ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));

  while(1)
  {
    ZTIMER_vTask();

    vAHI_WatchdogRestart();
  }
}
```

The `vAppMain()` function declares, initializes, and runs the timer. There can be many timers in the application, but the timers array must be allocated in advance. The app can basically init and de-init a specific timer in runtime, but the total number of timers can't exceed the timer array size.

Application main loop must call the `ZTIMER_vTask()` function which handles all the software timers. The function calls the user function back on timer expiration. In my case the timer will run every 1000 ms, and execute the `blinkFunc()` callback function.

```cpp
PUBLIC void blinkFunc(void *pvParam)
{
  static int iteration = 0;
  DBG_vPrintf(TRUE, "Blink iteration %d\n", iteration++);
 
  uint32 currentState = u32AHI_DioReadInput();
  vAHI_DioSetOutput(currentState^BOARD_LED_PIN, currentState&BOARD_LED_PIN);
}
```

There was some bitwise magic needed to implement LED toggle.

Timers part of the SDK is provided as source, and requires adding to the build.

```cmake
################################
# Common settings

ADD_DEFINITIONS(
  -DJENNIC_CHIP_NAME=_JN5169
  -DJENNIC_CHIP_FAMILY_NAME=_JN516x
  -DJENNIC_CHIP_FAMILY_JN516x
  -DJENNIC_CHIP_FAMILY=JN516x
  -DJN516x=5160
  -DDBG_ENABLE
  -DEMBEDDED
)


################################
# Zigbee Library

SET(ZIGBEE_SRC
  ${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZTimer.c
)

ADD_LIBRARY(ZigBee STATIC ${ZIGBEE_SRC})
TARGET_INCLUDE_DIRECTORIES(ZigBee PRIVATE
  ${SDK_PREFIX}/Components/PWRM/Include
)
```

Pay attention to how many new defines with the JN5169 word had to be added - it does not compile/work without them. I suspect that this is not all, and the list will be extended.

Running the code, and... As you may have guessed, nothing works. After a thorough study of the timer library (fortunately there are sources), it turns out that software timers are based on hardware tick timer interrupt. But inspecting the firmware binary showed that the interrupt code did not get into the firmware for some reason. That is interesting.

After some additional research I found the `irq_JN516x.S` file, which describes the interrupt vectors table for the microcontroller. But how does it get into the firmware? It is possible to compile this file, but since no one refers to this table, the linker simply throws it out of the firmware.

The only reference to the interrupt vectors table was gently wrapped in a piece of assembler code inside the `TARGET_INITIALISE()` macro. Hmmm... The name sounds reassuring, moreover this macro is called from `vAppMain()` in the example code. The dependency chain also includes the `portasm_JN516x.S` file from SDK, where the connection between `TARGET_INITIALIZE()` and the interrupt table was found, so this file shall be also added to the build.

Long story short, my `vAppMain()` just required the following piece of initialization code

```cpp
       // Initialize the hardware
        TARGET_INITIALISE();
        SET_IPL(0);
        portENABLE_INTERRUPTS();
```

As far as I understood from reading the [community.nxp.com](community.nxp.com) forum, editing the `irq_JN516x.S` file is a common approach for managing interrupts and their priorities. But I have not found anything about this in the documentation. Interestingly, as I learned from the example code, JN5179 (next generation MCU) interrupt management occurs directly in the code by calling the appropriate functions.

Let's upload the firmware. The LED blinked once and froze. What is the problem? After studying the timers code again (particularly `ZTIMER_vTask()` function) I realized that SDK provides only one-shot timers implementation. There simply is no code that reloads the timer for the next timer cycle. Although it is not a big deal to implement auto-reload timers, it could potentially break compatibility with ZigBee stack code, which also uses software timers and does not expect auto-reload behavior. It's easier to restart the timer in our handler.

```cpp
PUBLIC void blinkFunc(void *pvParam)
{
  static int iteration = 0;
  DBG_vPrintf(TRUE, "Blink iteration %d\n", iteration++);

  uint32 currentState = u32AHI_DioReadInput();
  vAHI_DioSetOutput(currentState^BOARD_LED_PIN, currentState&BOARD_LED_PIN);

  ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));
}
```

With addition of `ZTIMER_eStart()` call the LED started blinking as expected.

## Hello message queues

The next thing that I would like to deal with today is the message queues. Different firmware components can exchange messages with each other through queues. For example, let a short press on the user button change the blink rate of the LED, and a long press turn the blinking on/off entirely. Let's implement this using the queues.

I am going to launch 2 periodic tasks - one will poll the button, and the other will blink the LED. First of all we need to add button pin initialization in the `vAppMain()`, and enable a pull-up resistor for it. Also we need to launch another timer for button polling. And of course the most important thing - initialize the queue. I think the queue length of 3 elements is more than enough.

```cpp
 // Initialize hardware
 vAHI_DioSetDirection(BOARD_BTN_PIN, BOARD_LED_PIN);
 vAHI_DioSetPullup(BOARD_BTN_PIN, 0);

 // Init and start timers
 ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));
 ZTIMER_eOpen(&blinkTimerHandle, blinkFunc, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
 ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));
 ZTIMER_eOpen(&buttonScanTimerHandle, buttonScanFunc, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
 ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));

 // Initialize queue
 ZQ_vQueueCreate(&queueHandle, 3, sizeof(ButtonPressType), (uint8*)queue);
 ```

The button handler is called every 10ms, and counts the duration of the button press. Very short clicks (less than 50ms) will be discarded - treating this as debouncing. We will also distinguish between short and long (over 2 seconds) presses, and send the corresponding messages to the queue.

```cpp
typedef enum
{
    BUTTON_SHORT_PRESS,
    BUTTON_LONG_PRESS
} ButtonPressType;


PUBLIC void buttonScanFunc(void *pvParam)
{
    static int duration = 0;

    uint32 input = u32AHI_DioReadInput();
    bool btnState = (input & BOARD_BTN_PIN) == 0;

    if(btnState)
    {
        duration++;
    }
    else
    {
        // detect long press
        if(duration > 200)
        {
            DBG_vPrintf(TRUE, "Button released. Long press detected\n");
            ButtonPressType value = BUTTON_LONG_PRESS;
            ZQ_bQueueSend(&queueHandle, (uint8*)&value);
        }

        // detect short press
        else if(duration > 5)
        {
            DBG_vPrintf(TRUE, "Button released. Short press detected\n");
            ButtonPressType value = BUTTON_SHORT_PRESS;
            ZQ_bQueueSend(&queueHandle, &value);
        }

        duration = 0;
    }

    ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
}
```

Let's look at the blink function. Depending on the received message from the queue, the blinking parameters change - fast/slow, on/off. if it is on, then we blink at the required speed.

```cpp
PUBLIC void blinkFunc(void *pvParam)
{
    static bool fastBlink = true;
    static bool enabled = true;

    ButtonPressType value;    
    if(ZQ_bQueueReceive(&queueHandle, (uint8*)&value))
    {
        if(value == BUTTON_SHORT_PRESS)
            fastBlink = !fastBlink;

        if(value == BUTTON_LONG_PRESS)
            enabled = !enabled;
    }

    if(enabled)
    {
        uint32 currentState = u32AHI_DioReadInput();
        vAHI_DioSetOutput(currentState^BOARD_LED_PIN, currentState&BOARD_LED_PIN);
    }

    ZTIMER_eStart(blinkTimerHandle, fastBlink ? ZTIMER_TIME_MSEC(200) : ZTIMER_TIME_MSEC(1000));
}
```

This code behaves as expected - user can switch between fast and slow blinking, as well as turn on/off blinking.

## Hello DIO Interrupts

Polling buttons in an active cycle is cool and simple, but is not optimal from a power saving perspective. For battery powered devices, this may not be acceptable. Let's try to move away from the polling the button, and instead use the interrupt on the falling edge on the corresponding pin.

This is what the initialization will look like now.

```cpp
 // Initialize hardware
 vAHI_DioSetDirection(BOARD_BTN_PIN, BOARD_LED_PIN);
 vAHI_DioSetPullup(BOARD_BTN_PIN, 0);
 vAHI_DioInterruptEdge(0, BOARD_BTN_PIN);
 vAHI_DioInterruptEnable(BOARD_BTN_PIN, 0);

 // Init and start timers
 ZTIMER_eInit(timers, sizeof(timers) / sizeof(ZTIMER_tsTimer));
 ZTIMER_eOpen(&blinkTimerHandle, blinkFunc, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
 ZTIMER_eStart(blinkTimerHandle, ZTIMER_TIME_MSEC(1000));
 ZTIMER_eOpen(&buttonScanTimerHandle, buttonScanFunc, NULL, ZTIMER_FLAG_PREVENT_SLEEP);
 //ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
```

Edge interrupt is enabled by the vAHI_DioInterruptEdge() and vAHI_DioInterruptEnable() functions. Please note that I commented out the button polling timer start, but the ZTIMER_eOpen() timer initialization function is still needed.

Unlike ATMega or STM32, in JN5169 many events come to the same interrupt handler - `vISR_SystemController()`. It handles not only pin change interrupts, but also comparator, supply voltage change, waking up on a timer, and something else. You can find out what exactly has happened by calling the appropriate functions.

```cpp
PUBLIC void vISR_SystemController(void)
{
    DBG_vPrintf(TRUE, "In vISR_SystemController\n");

    if(u32AHI_DioInterruptStatus() & BOARD_BTN_PIN)
    {
        DBG_vPrintf(TRUE, "Button interrupt\n");

        ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
    }
}
```

We are interested in the DIO Interrupt event. This event will occur when the button is clicked (and a pin state is changed). At this point it is safe to start the polling timer, which will poll the pin until the button is released.

Button polling has changed only slightly. Now the timer is not always restarted, but only while the button is pressed.

```cpp
PUBLIC void buttonScanFunc(void *pvParam)
{
    static int duration = 0;

    uint32 input = u32AHI_DioReadInput();
    bool btnState = (input & BOARD_BTN_PIN) == 0;

    if(btnState)
    {
        duration++;
        DBG_vPrintf(TRUE, "Button still pressed for %d ticks\n", duration);
        ZTIMER_eStart(buttonScanTimerHandle, ZTIMER_TIME_MSEC(10));
    }
    else
    {
        // detect long press
        if(duration > 200)
        {
            DBG_vPrintf(TRUE, "Button released. Long press detected\n");
            ButtonPressType value = BUTTON_LONG_PRESS;
            ZQ_bQueueSend(&queueHandle, (uint8*)&value);
        }

        // detect short press
        else if(duration > 10)
        {
            DBG_vPrintf(TRUE, "Button released. Short press detected\n");
            ButtonPressType value = BUTTON_SHORT_PRESS;
            ZQ_bQueueSend(&queueHandle, &value);
        }

        duration = 0;
    }
}
```

Same as with Tick Timer interrupts, the `vISR_SystemController()` function must be added to the interrupt vector table in the `irq_JN516x.S` file. Overall this file is stolen from the example code.

```asm
.globl  PIC_ChannelPriorities
    .section .text,"ax"
    .align 4
    .type   PIC_ChannelPriorities, @object
    .size   PIC_ChannelPriorities, 16
PIC_ChannelPriorities:
    .byte 0                 # pwm1 priority
    .byte 0                 # pwm2 priority
    .byte 1                 # system controller priority
    .byte 7                 # MAC priority
    .byte 0                 # AES priority
    .byte 0                 # PHY priority
    .byte 0                 # uart0 priority
    .byte 0                 # uart1 priority
    .byte 0                 # timer0 priority
    .byte 0                 # spi slave priority
    .byte 0                 # i2c maste/slave priority
    .byte 0                 # spi master priority
    .byte 0                 # pwm4 priority
    .byte 0                 # analog peripherals priority
    .byte 0                 # pwm3 priority
    .byte 15                # tick timer priority


.globl  PIC_SwVectTable
    .section .text,"ax"
    .extern zps_isrMAC
    .extern ISR_vTickTimer
    .extern vISR_SystemController
    .align 4
    .type   PIC_SwVectTable, @object
    .size   PIC_SwVectTable, 64
PIC_SwVectTable:
    .word vUnclaimedInterrupt               # 0
    .word vISR_SystemController             # 1
    .word vUnclaimedInterrupt               # 2
    .word vUnclaimedInterrupt               # 3
    .word vUnclaimedInterrupt               # 4
    .word vUnclaimedInterrupt               # 5
    .word vUnclaimedInterrupt               # 6
#    .word zps_isrMAC                        # 7
    .word vUnclaimedInterrupt               # 7
    .word vUnclaimedInterrupt               # 8
    .word vUnclaimedInterrupt               # 9
    .word vUnclaimedInterrupt               # 10
    .word vUnclaimedInterrupt               # 11
    .word vUnclaimedInterrupt               # 12
    .word vUnclaimedInterrupt               # 13
    .word vUnclaimedInterrupt               # 14
    .word ISR_vTickTimer                    # 15
```

Not sure I understand and can explain what is going on in this assembly list. Let's just use it as is. The main goal is achieved - it detects pin change and can handle this appropriately.

## Summary

Software timers and queues are pretty nice building blocks that are widely used inside Zigbee stack, and also offered to the user application. Despite some configuration difficulties with hardware interrupts, these components look easy to use.

In this article I also described usage of GPIO (DIO) interrupts and how they can be used to interact with timers and queues.

## Links:

- [Project on github](https://github.com/grafalex82/hellojn5169world)
- https://www.nxp.com/docs/en/user-guide/JN-UG-3113.pdf
- https://www.nxp.com/docs/en/user-guide/JN-UG-3116.pdf
- https://www.nxp.com/docs/en/user-guide/JN-UG-3087.pdf
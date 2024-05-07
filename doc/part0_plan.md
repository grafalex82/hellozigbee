# Hello Zigbee World, Part 0 — Motivation, Goals, Plan

## My story

Many years ago I was reading Bill Gates' "The Road Ahead" book. The book described a concept of a smart house, where lighting, temperature, and even ambient music was controlled with a computer algorithm. The smart home could adjust to the tenants' schedule, or even mood, providing maximum comfort and convenience. The only problem is that this system was very expensive in those years.

The situation changed around 2018, when I found a youtube video presenting Xiaomi Aqara smart home kit. I was an experienced engineer by that time, and possibly could create some Arduino based solutions for smart homes, but this required a lot of work. At the same time Xiaomi devices provided a ready to use solution out of the box. Unlike Arduino based devices that looked like a clew of wires, Aqara ones had a pretty nice design, and devices were factory produced. Moreover the starting kit did cost less than $100.

![](images/aqara_QBKG12LM.jpg)
<p><figcaption align = "center"><i>Xiaomi Aqara QBKG12LM smart switch</i></figcaption></p>

I bought a couple of Xiaomi Aqara switches and several sensors, and I began to enjoy life in a slightly smarter apartment. Later I switched to [zigbee2mqtt](https://www.zigbee2mqtt.io/) plus a CC2538 stick, which allowed me to integrate devices from other manufacturers to my system. Overall, I was inspired with confidence in the future of the technology. When moving to a bigger apartment, I ordered a dozen more Xiaomi Aqara switches and looked forward to how cool it would work on a scale. I was so naive...

The bummer came in the first week after the move. Red error lines started appearing in zigbee2mqtt logs. Switches strangely and randomly fell off the network, stopped responding to Z2M commands, as well as sending their status to the system. Switches themselves basically continued working as switches, but no longer were controlled by the system. Some switches could turn off spontaneously, and sometimes turn on when you try to turn them off. But the most annoying thing was that some of the temperature and occupancy sensors near the switch also disappeared from the network. Power juggling didn't help. Once a day, I had to make an evening round and re-join the switches that had fallen off — only this could bring them back to life. After some experiments I was pretty sure that the problem was in switches, but other device types may have issues as well.

It appears that this problem is known, and I could just buy switches from another manufacturer which do not have this issue. Moreover Xiaomi presented new versions of their switches, that probably have this problem fixed as well. It would cost me a few more hundred dollars and forget about the problem.

But I decided to go with a longer, but much more interesting way. I wanted to get a better understanding of what is going on, and possibly fix the problem myself. I uncovered the sniffer, and started diving into the ZigBee. Xiaomi devices are based on the NXP JN5169 microcontroller, so I started looking at those datasheets as well. The result of the dive was the reverse engineering of the device, and some confidence that I could write an alternative firmware which would fix mentioned issues.

## The Goals

As I just said the **goal** is to develop an alternative firmware for Xiaomi Aqara QBKG12LM switches. This is the first model of Aqara smart switches, has two buttons and a neutral line. Besides its connectivity issues, I lacked a few modern features. So I state the following list of features of an ideal QBKG12LM switch firmware.

- Stable operations in the large Zigbee network
- Possibility to bind switch channels to other devices (e.g. relay switches or lamps)
- More precise control on single/double/long clicks
- More precise control on decoupling internal relay switch from the buttons
- Integration with zigbee2mqtt
- Keeping other functions (temperature and power measurements) working

Obviously this requires a deep dive into Zigbee networks, and a good understanding of how this works under the hood. Having this experience I could also work on other devices' firmware, such as temperature, light and occupancy sensors, power measurement devices. Zigbee knowledge could be also helpful when developing firmware for other Zigbee microcontrollers used in other manufacturers' devices.

Unfortunately I have not found a simple Zigbee tutorial on the internet. There are a few projects like "I have an alternative firmware — use it". There are also a few open source projects that provide some firmware skeleton with a short instruction on how to extend it. But these projects do not provide me enough confidence in the technology.

So here I come to an **ultimate goal** - create an extendable platform for creating Zigbee devices. This ambitious goal includes:

- Creating easy to understand example code
- Creating a set of articles and tutorials that explain Zigbee fundamentals, as well as the code
- Share articles with DIY community
- Ideally this would include not just NXP JN5169 microcontroller, but possibly other MCUs as well

## The Plan

The QBKG12LM switch is a quite complex device. It has a lot of on board circuits that I do not fully understand at the moment. So there is a quite high risk to do something wrong, and break the device. Moreover, this is a line powered device with a high voltage line, so there is also a risk of injury, or damaging a connected computer.

I was also looking at a development board based on the same NXP JN5169 microcontroller. But the [official development board from NXP](https://www.nxp.com/products/wireless/zigbee/zigbee-evaluation-kit-with-nfc-commissioning:JN5169HA) costs \$650, which is too expensive for experiments. Fortunately there is a E75-2G4M10S module from EBYTE that costs just a few dollars. I bought several modules for around \$3 each. So it is quite cheap for experiments, and I would not get upset if I burn a few. The only problem is it is just a module, and requires some soldering to make a circuit. But this is not a big deal.

![](images/E75-2G4M10S.jpg)
<p><figcaption align = "center"><i>EBYTE E75-2G4M10S module</i></figcaption></p>

Next big problem is that the examples provided by NXP are too complicated for a newbie, and poorly described in the documentation. Usually when I learn a new technology I am working in small increments, attacking one problem at a time. Moreover the best approach to get a deep understanding of a technology is to start a simple project from scratch, and add features one by one. This allows me to control each step, and quickly get back to a previous step if something goes wrong.

Before I can get to the real Xiaomi hardware, I would like to practice on that module first. For this purpose I'll try to build a simple device that would act like a smart switch. This will allow to develop all the building blocks that could be helpful for the real device firmware:

- Join the network and get properly identified by zigbee2mqtt
- A few buttons that will generate zigbee switch commands
- React on on/off commands and light a corresponding LED
- Handle single/double/long button presses
- Binding of the switch with other Zigbee devices
- OTA firmware update
- Custom zigbee clusters for controlling the device settings
- Temperature and power measurements, that are available in the Xiaomi device
- All these additional capabilities shall be integrated with zigbee2mqtt

When this is done I can easily port this to the real Xiaomi hardware. Moreover these building blocks will be also helpful for creating firmware for other Xiaomi devices.

After some experimenting, and having some progress on the project, I can now define a plan.

**Stage 1** — get a working device using E75–2G4M10S module as a standalone device
- Solder a simple circuit with a few buttons and LEDs ([Article](part1_bring_up.md))
- Set up the toolchain, that can compile the firmware ([Article](part1_bring_up.md))
- Write a simple application, e.g. LED blinker ([Article](part1_bring_up.md))
- Get understanding of how to flash the device ([Article](part1_bring_up.md))
- Learn basic peripherals, that could be used in the project e.g. GPIO and UART ([Article](part1_bring_up.md))
- Use watchdog timers ([Article](part1_bring_up.md))
- Understand building blocks provided by SDK, e.g. software timers and message queues ([Article](part2_timers_queues.md))
- Understand power saving options and sleep modes ([Article](part3_sleep_modes.md))

**Stage 2** — get a working device as a Zigbee network node
- Get basic understanding of Zigbee Network and SDK ([Article](part4_zigbee_basics.md))
- Initialize Zigbee stack ([Article](part5_zigbee_init.md))
- Join the network ([Article](part6_join_zigbee_network.md))
- Register device in zigbee2mqtt ([Article](part7_zigbee_descriptors.md))
- Implement a basic Zigbee smart switch functionality ([Article](part8_simple_switch.md))
- Handle re-joining the network in case of network loss ([Article](part10_joining_rejoining.md))
- Developing an end device ([Article](part11_end_device.md))
- Handle join/rejoin for the end device ([Article](part12_end_device_rejoin.md))
- Implement direct binding to other devices ([Article](part14_reports_binding.md))
- Understand the difference between reports and commands ([Article](part15_commands_binding.md))
- Handle single/double/long button presses using Multistate Input cluster ([Article](part16_multistate_action.md))
- Create a custom Zigbee cluster in order to expose device settings ([Article](part17_custom_cluster.md))
- Make a custom zigbee2mqtt external converter to handle the custom cluster ([Article](part18_zigbee2mqtt_converter.md))
- Dimming light support using Level Control Client cluster ([Article](part19_level_control.md))
- Double buttons support ([Article](part24_misc_improvements.md))
- Implement OTA firmware update from the device side ([Article](part25_ota_updates.md))

**Stage 3** — nice to have stuff
- Using C++ in the project ([Article](part9_cpp_building_blocks.md))
- Extendable structure of the project codebase ([Article](part13_project_cpp_structure.md))
- Understand how device flashing works ([Article](part27_flash_tool.md))
- Remote flashing of the device mounted on the wall, alternative to OTA update (Code ready, article pending)
- Build a Dimmable Light via Level Ctrl Server cluster ([Article](part20_dimmable_light.md))
- Build comprehensive automated test suit ([Article](part21_test_automation.md))
- Explore automatic attributes reporting feature ([Article](part24_misc_improvements.md))
- Explore Identify cluster ([Article](part22_identify_cluster.md))
- Explore Groups ([Article](part23_groups.md))
- Explore Scenes cluster
- Explore Touchlink, Install Codes, Find-And-Bind, and ZLL Commisioning
- Set up CI (Done)

**Stage 4** — porting to the real Xiaomi QBKG12LM hardware
- Reverse engineer the schematics ([Article](part26_QBKG12LM_support.md))
- Port maing functionality to the device - joining/leaving the network, switch functions, Drive LEDs and relays, OTA updates ([Article](part26_QBKG12LM_support.md))
- Handle temperature sensor via Device Temperature Configuration Cluster (Code ready, article pending)
- Handle current and power sensor
- Test the device in house ([Article](part28_remote_logger.md))
- Share the firmware with Smart Home community for a wider testing
- Explore possibility to extend the firmware to other devices, such as QBKG03LM/QBKG04LM (similar switches without a neutral line), and zigbee relays.

**Stage 5** — evaluate other MCUs
- Evaluate NXP JN5189 (ARM based one)
- Evaluate possibility to use modern C++ compilers
- Evaluate possibility to use FreeRTOS
- Evaluate Texas Instruments chips
- Evaluate BL702 chip

The half of the project is already done. Usually some learning, coding and experiments go first, and when I have enough material I write an article. So the articles go slightly behind the code. So stay tuned.
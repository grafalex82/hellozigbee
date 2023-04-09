# Hello NXP JN5169 ZigBee World

This is a tiny example how to work with ZigBee stack using JN5169 microcontroller. The example implements a smart switch with the following features:
- Zigbee router device
- 2 button channels, each of them offers
	- Handling button according to selected mode
	- On/Off zigbee cluster to report state change, as well as handling toggle commands
	- On/Off Configuraiton cluster to configure device behavior
	- Multistate input cluster to report single/double/triple/long press actions
- binding support
- OTA firmware update

There are 2 goals of the project:
- build an alternate firmware for Xiaomi Aqara QBKG12LM Zigbee smart switch
- learn how Zigbee works, and [describe this in a form of tutorial](part0_plan.md), so that more people can join the technology


# Test board

Basically the code is almost independent of the hardware (assuming it is based on JN5169). I am using a simple schematics based on a cheap EBYTE E75-2G4M10S module.

![Schematics](doc/images/Schematics2.png)

# How to build

Prerequisites:
- Beyond Studio IDE (comes with a JN5169 compiler)
- ZigBee SDK (JN-SW-4170 Zigbee 3.0 v1840.zip)
- CMake (any recent one)
- MinGW (or other source where you can get `make`)
- This all is Windows only

Build instructions:
- Clone the repo
- make a `build` directory
- `cd build`
- `cmake -G "MinGW Makefiles" -DTOOLCHAIN_PREFIX=C:/NXP/bstudio_nxp/sdk/Tools/ba-elf-ba2-r36379 -DSDK_PREFIX=C:/NXP/bstudio_nxp/sdk/JN-SW-4170 ..`
(Correct paths to the toolchain and sdk if needed)
- mingw32-make HelloZigbee.bin

Flash instructions:
- Open Beyond Studio
- Put the device in the programming mode (drive SPI_MISO low while reset or power up)
- Go to Device->Program Device
- Select the built HelloWorld.bin file
- Click `Program` button

or

- Put the device in the programming mode (drive SPI_MISO low while reset or power up)
- mingw32-make HelloZigbee.flash

# How to use

- The device is implementing a common type of home automation devices.
	- On the first start it is not connected to the network
	- Make sure your network permits joining, otherwise the device is not able to join
	- Press a button for 5 seconds to connect to the network
	- Press button for another 5 seconds to force the device to leave the network.
- Device is automatically try to rejoin the network if network conditions change (e.g. parent/neighbour router no longer respond)
	- Router device will find a way to a coordinator using a different router
	- End device will search for another router and send rejoin request to the new parent.
- Device joining and rejoining, as well as failure recovery is implemented using BDB component (a part of Zigbee SDK).
	- Firmware adds just a few rejoin attemts on top of standard BDB implementation
- Device supports binding commands that come from the coordinator (e.g. zigbee2mqtt). It supports binding of On/Off cluster.
	- If the device is not bound to any other device - on button press it sends reports to the coordinator
	- If the device is bound to another device - on button press it sends Toggle command to the bound device(s)

# Zigbee2mqtt integration

By default zigbee2mqtt will be able to interview the device, but it will list the device as unsupported. No features will be exposed. 

To integrate the device follow these steps:
- Put myswitch.js to zigbee2mqtt configuration folder (next to configuration.yaml)
- In configuration.yaml add this entity
```
external_converters:
  - myswitch.js
```

# Documentation

All this code is explained in very detail in the [Hello Zigbee article series](doc/part0_plan.md)

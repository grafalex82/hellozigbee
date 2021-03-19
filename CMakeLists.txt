SET(CMAKE_BUILD_TYPE Release)
SET(CMAKE_TOOLCHAIN_FILE JN5169.cmake)

PROJECT(GPSLogger)
CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
ENABLE_LANGUAGE(CXX)
ENABLE_LANGUAGE(ASM)

IF(NOT SDK_PREFIX)
     MESSAGE(FATAL_ERROR "No SDK_PREFIX specified")
ENDIF()


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
	-DPDM_NO_RTOS
)


################################
# Zigbee Library

SET(ZIGBEE_SRC
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZTimer.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZQueue.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/portasm_JN516x.S
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/port_JN516x.c
)
ADD_LIBRARY(ZigBee STATIC ${ZIGBEE_SRC})
TARGET_INCLUDE_DIRECTORIES(ZigBee PRIVATE
	${SDK_PREFIX}/Components/PWRM/Include
	${SDK_PREFIX}/Components/MicroSpecific/Include
)


################################
# Hello World

INCLUDE_DIRECTORIES(
	${SDK_PREFIX}/Components/Common/Include
	${SDK_PREFIX}/Components/Dbg/Include
	${SDK_PREFIX}/Components/HardwareAPI/Include
	${SDK_PREFIX}/Components/ZigbeeCommon/Include
	${SDK_PREFIX}/Components/MicroSpecific/Include
	${SDK_PREFIX}/Components/PWRM/Include
	${SDK_PREFIX}/Components/PDM/Include
	${SDK_PREFIX}/Components/Aes/Include
)

LINK_DIRECTORIES(
	${SDK_PREFIX}/Components/Library
	${SDK_PREFIX}/Chip/JN5169/Build
	${SDK_PREFIX}/Platform/DK4/Library
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -TAppBuildZBPro.ld")

SET(SOURCES
	Main.c
	irq_JN516x.S
)

ADD_EXECUTABLE(XiaomiSwitch ${SOURCES})
TARGET_LINK_LIBRARIES(XiaomiSwitch
        -Wl,--start-group

	ZigBee	

        PWRM_JN516x
	HardwareApi_JN5169
	DBG_JN516x
	Boot_JN516x
        PDM_EEPROM_NO_RTOS_JN516x
        MicroSpecific_JN516x

        -Wl,--end-group
)

STM32_PRINT_SIZE_OF_TARGETS(XiaomiSwitch)
STM32_ADD_HEX_BIN_TARGETS(XiaomiSwitch)
STM32_ADD_DUMP_TARGET(XiaomiSwitch)

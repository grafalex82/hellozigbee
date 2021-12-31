
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
        -DJENNIC_MAC_MiniMacShim
        -DOTA_INTERNAL_STORAGE


#        -DTRACE_ZCL=TRUE
        -DTRACE_OTA_DEBUG=TRUE
#        -DDEBUG_BDB
#        -DDEBUG_CLD_ONOFF
#        -DDEBUG_ZQUEUE

        -DBDB_SUPPORT_NWK_STEERING
)


################################
# Zigbee Library

SET(ZIGBEE_SRC
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/app_zps_link_keys.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/appZdpExtraction.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/appZpsBeaconHandler.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZTimer.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZQueue.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/portasm_JN516x.S
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/port_JN516x.c

        ${SDK_PREFIX}/Components/ZCIF/Source/dlist.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_attribute.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_buffer.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_command.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_common.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_CustomCommandReceive.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_CustomCommandSend.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_defaultResponse.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_event.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_heap.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_library_options.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_PDUbufferReadWrite.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_PDUbufferReadWriteString.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_readAttributesRequestHandle.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_readAttributesResponseHandle.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_reportManager.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_reportMaths.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_reportStringHandling.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_reportStructure.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_search.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_timer.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_transmit.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_WriteAttributesRequestHandle.c

        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Alarms.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Basic.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/BasicClientCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/BasicCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Identify.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyClientCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyServerCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/MultistateInputBasic.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/OnOff.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/OnOffCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/OnOffCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Scenes.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/ScenesCommandHandler.c

        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA_client.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA_ClientUpgradeManager.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA_common.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA_CustomCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA_CustomReceiveCommands.c

        ${SDK_PREFIX}/Components/BDB/Source/Common/bdb_fr.c
        ${SDK_PREFIX}/Components/BDB/Source/Common/bdb_start.c
        ${SDK_PREFIX}/Components/BDB/Source/Common/bdb_state_machine.c
        ${SDK_PREFIX}/Components/BDB/Source/NwkSteering/bdb_ns.c
)
ADD_LIBRARY(ZigBee STATIC ${ZIGBEE_SRC})
TARGET_INCLUDE_DIRECTORIES(ZigBee PRIVATE
        ${SDK_PREFIX}/Components/BDB/Include
        ${SDK_PREFIX}/Components/BDB/Source/Common
        ${SDK_PREFIX}/Components/BDB/Source/NwkSteering
        ${SDK_PREFIX}/Components/PWRM/Include
	${SDK_PREFIX}/Components/MicroSpecific/Include
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source
        ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Include

        ${SDK_PREFIX}/Components/ZCIF/Source

        # Zigbee sources reference zcl_options.h
        ${PROJECT_SOURCE_DIR}/src
)


################################
# Hello World

INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}

    ${SDK_PREFIX}/Components/Aes/Include
    ${SDK_PREFIX}/Components/AES_SW/Include
    ${SDK_PREFIX}/Components/AppApi/Include

    ${SDK_PREFIX}/Components/BDB/Include

    ${SDK_PREFIX}/Components/Common/Include
    ${SDK_PREFIX}/Components/Dbg/Include
    ${SDK_PREFIX}/Components/HardwareAPI/Include
    ${SDK_PREFIX}/Components/MAC/Include
    ${SDK_PREFIX}/Components/MicroSpecific/Include
    ${SDK_PREFIX}/Components/MiniMAC/Include
    ${SDK_PREFIX}/Components/MMAC/Include
    ${SDK_PREFIX}/Components/PDM/Include
    ${SDK_PREFIX}/Components/PDUM/Include
    ${SDK_PREFIX}/Components/PWRM/Include
    ${SDK_PREFIX}/Components/Random/Include

    ${SDK_PREFIX}/Components/TimerServer/Include

    ${SDK_PREFIX}/Components/ZCIF/Include
    ${SDK_PREFIX}/Components/ZCL/Clusters/General/Include
    ${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Include
    ${SDK_PREFIX}/Components/ZCL/Clusters/MeasurementAndSensing/Include
    ${SDK_PREFIX}/Components/ZCL/Devices/ZLO/Include
    ${SDK_PREFIX}/Components/ZigbeeCommon/Include
    ${SDK_PREFIX}/Components/ZPSAPL/Include
    ${SDK_PREFIX}/Components/ZPSMAC/Include
    ${SDK_PREFIX}/Components/ZPSNWK/Include
    ${SDK_PREFIX}/Components/ZPSTSV/Include

    # Zigbee sources reference zcl_options.h
    ${PROJECT_SOURCE_DIR}/src
)

LINK_DIRECTORIES(
	${SDK_PREFIX}/Components/Library
	${SDK_PREFIX}/Chip/JN5169/Build
	${SDK_PREFIX}/Platform/DK4/Library
)

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${HelloZigbee_SOURCE_DIR}/src/HelloZigbee.ld")

SET(SOURCES
	irq_JN516x.S
        pdum_gen.c
        pdum_apdu.S
        zps_gen.c
        Queue.h
        Timer.h
        PeriodicTask.h
        PersistedValue.h
        ButtonModes.h
        PdmIds.h
        AppQueue.cpp
        BlinkTask.cpp
        ButtonsTask.cpp
        ButtonHandler.cpp
        PollTask.cpp
        DeferredExecutor.cpp
        DumpFunctions.cpp
        Endpoint.cpp
        SwitchEndpoint.cpp
        EndpointManager.cpp
        ZigbeeUtils.cpp
        ZigbeeDevice.cpp
        BasicClusterEndpoint.cpp
	OOSC.c
        OTAHandlers.cpp
        ZCLTimer.cpp
        Main.cpp
)

ADD_EXECUTABLE(HelloZigbee ${SOURCES})
TARGET_LINK_LIBRARIES(HelloZigbee
        -Wl,--start-group

	ZigBee	

        Aes_JN516x
        AES_SW_JN516x
#        AppApi_JN5169
        Boot_JN516x
        DBG_JN516x
        HardwareApi_JN5169
        JPT_JN5169
#        MAC_JN5169
        MicroSpecific_JN516x
        MiniMac_JN5169
        MiniMacShim_JN516x
        MMAC_JN5169
        PDM_EEPROM_NO_RTOS_JN516x
        PDUM_JN516x
        Random_JN516x
        PWRM_JN516x
        ZPSAPL_JN516x
        ZPSAPL_ZCP-GU_JN516x
        ZPSMAC_Mini_SOC_JN516x
        ZPSNWK_JN516x
        ZPSTSV_JN516x

        -Wl,--end-group
)

PRINT_SIZE_OF_TARGETS(HelloZigbee)
ADD_HEX_BIN_TARGETS(HelloZigbee)
ADD_DUMP_TARGET(HelloZigbee)
FLASH_FIRMWARE_TARGET(HelloZigbee)
ADD_OTA_BIN_TARGETS(HelloZigbee)

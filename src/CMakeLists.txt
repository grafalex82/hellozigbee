################################
# Common settings

add_definitions(
        -DTARGET_BOARD_${BOARD}

        -DBUILD_DATE="${BUILD_DATE}"
        -DBUILD_DATE_LEN=${BUILD_DATE_LEN}
        -DVERSION_STR="${VERSION_STR}"
        -DVERSION_STR_LEN=${VERSION_STR_LEN}

        -DOTA_INTERNAL_STORAGE
#        -DTRACE_ZCL=TRUE
#        -DTRACE_OTA_DEBUG=TRUE
        -DDEBUG_CLD_GROUPS
#        -DDEBUG_BDB
        -DDEBUG_CLD_ONOFF
#        -DDEBUG_ZQUEUE
#        -DDEBUG_CLD_IDENTIFY

        -DBDB_SUPPORT_NWK_STEERING
)

################################
# Generated files (used by both ZigbeeLibrary and the app)
generate_zps_and_pdum_targets(${PROJECT_SOURCE_DIR}/src/HelloZigbee.zpscfg)

################################
# Zigbee Library

set(ZIGBEE_SRC
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/app_zps_link_keys.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/appZdpExtraction.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/appZpsBeaconHandler.c
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZTimer.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/ZQueue.c
	${SDK_PREFIX}/Components/ZigbeeCommon/Source/portasm_${JENNIC_CHIP_FAMILY}.S
        ${SDK_PREFIX}/Components/ZigbeeCommon/Source/port_${JENNIC_CHIP_FAMILY}.c

        ${SDK_PREFIX}/Components/ZCIF/Source/dlist.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_attribute.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_buffer.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_command.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_common.c
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_configureReportingCommandHandle.c        
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
        ${SDK_PREFIX}/Components/ZCIF/Source/zcl_reportScheduler.c
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
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/DeviceTemperatureConfiguration.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Groups.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/GroupsClientCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/GroupsCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/GroupsServerCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/GroupsTableManager.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/Identify.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyClientCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyCommandHandler.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/IdentifyServerCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/LevelControl.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/LevelControlClientCommands.c
        ${SDK_PREFIX}/Components/ZCL/Clusters/General/Source/LevelControlCommandHandler.c
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

        # Generated (used by bdb_ns.c)
        pdum_gen.h
)
add_library(ZigBee STATIC ${ZIGBEE_SRC})
set_source_files_properties(${SDK_PREFIX}/Components/ZCL/Clusters/OTA/Source/OTA.c PROPERTIES COMPILE_FLAGS -Wno-unused-variable)
target_include_directories(ZigBee PRIVATE
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
# Hello Zigbee

include_directories(
    ${PROJECT_SOURCE_DIR}

    ${SDK_PREFIX}/Components/Aes/Include
    ${SDK_PREFIX}/Components/AES_SW/Include
    ${SDK_PREFIX}/Components/AppApi/Include

    ${SDK_PREFIX}/Components/BDB/Include

    ${SDK_PREFIX}/Components/Common/Include
    ${SDK_PREFIX}/Components/DBG/Include
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

link_directories(
	${SDK_PREFIX}/Components/Library
	${SDK_PREFIX}/Chip/${JENNIC_CHIP}/Build
	${SDK_PREFIX}/Platform/DK4/Library
)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T${HelloZigbee_SOURCE_DIR}/src/HelloZigbee.ld")

set(SOURCES
	irq_JN516x.S
        Queue.h
        Timer.h
        PeriodicTask.h
        PersistedValue.h
        ButtonModes.h
        PdmIds.h
        GPIOPin.h
        PWMPin.h
        BlinkTask.cpp
        ButtonsTask.cpp
        DebugInput.cpp
        LEDTask.cpp
        LEDHandler.cpp
        LEDPair.cpp
        RelayHandler.cpp
        RelayTask.cpp
        ButtonHandler.cpp
        PollTask.cpp
        DumpFunctions.cpp
        Endpoint.cpp
        SwitchEndpoint.cpp
        EndpointManager.cpp
        ZigbeeDevice.cpp
        BasicClusterEndpoint.cpp
	OOSC.c
        OTAHandlers.cpp
        ZCLTimer.cpp
        Main.cpp

        # Generated by generate_zps_and_pdum_targets()
        zps_gen.c
        pdum_gen.c
        pdum_apdu.S
)

add_executable(HelloZigbee ${SOURCES})
target_link_libraries(HelloZigbee
        -Wl,--start-group

	ZigBee	

        Aes_${JENNIC_CHIP_FAMILY}
        AES_SW_${JENNIC_CHIP_FAMILY}
        Boot_${JENNIC_CHIP_FAMILY}
        DBG_${JENNIC_CHIP_FAMILY}
        HardwareApi_${JENNIC_CHIP}
        JPT_${JENNIC_CHIP}
        MicroSpecific_${JENNIC_CHIP_FAMILY}
        MiniMac_${JENNIC_CHIP}
        MiniMacShim_${JENNIC_CHIP_FAMILY}
        MMAC_${JENNIC_CHIP}
        PDM_EEPROM_NO_RTOS_${JENNIC_CHIP_FAMILY}
        PDUM_${JENNIC_CHIP_FAMILY}
        Random_${JENNIC_CHIP_FAMILY}
        PWRM_${JENNIC_CHIP_FAMILY}
        ZPSAPL_${JENNIC_CHIP_FAMILY}
        ZPSAPL_ZCP-GU_${JENNIC_CHIP_FAMILY}
        ZPSMAC_Mini_SOC_${JENNIC_CHIP_FAMILY}
        ZPSNWK_${JENNIC_CHIP_FAMILY}
        ZPSTSV_${JENNIC_CHIP_FAMILY}

        -Wl,--end-group
)

print_size_of_targets(HelloZigbee)
add_hex_bin_targets(HelloZigbee)
add_dump_target(HelloZigbee)
add_flash_firmware_target(HelloZigbee)
add_ota_bin_target(HelloZigbee)

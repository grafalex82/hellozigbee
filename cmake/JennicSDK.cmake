# Check mandatory parameters
if(NOT SDK_PREFIX)
    set(SDK_PREFIX "${CMAKE_SOURCE_DIR}/sdk" CACHE PATH "Path to the root of JN-SW-4170 SDK")
endif()

if(NOT TOOLCHAIN_PREFIX)
    message(FATAL_ERROR "No TOOLCHAIN_PREFIX specified (it must point to the root of desired compiler bundle)")
endif()

if(NOT JENNIC_CHIP)
    message(FATAL_ERROR "No JENNIC_CHIP specified (it must reflect target chip name)")
endif()

# Correct SDK prefix so that it is absolute path
if(NOT IS_ABSOLUTE ${SDK_PREFIX})
    set(SDK_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${SDK_PREFIX}")
endif()
get_filename_component(SDK_PREFIX ${SDK_PREFIX} ABSOLUTE)
message(STATUS "Using SDK path: ${SDK_PREFIX}")

# Load the toolchain file for a selected chip
get_filename_component(JENNIC_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${JENNIC_CMAKE_DIR} ${CMAKE_MODULE_PATH})

if(JENNIC_CHIP STREQUAL "JN5169")
    set(CMAKE_TOOLCHAIN_FILE ${JENNIC_CMAKE_DIR}/${JENNIC_CHIP}.cmake)     
    message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")

    set(JENNIC_CHIP_FAMILY "JN516x")
else()
    message(FATAL_ERROR "Unsupported target chip - ${JENNIC_CHIP}")
endif()


# Set build parameters common for the app and Zigbee library
add_definitions(
	-DJENNIC_CHIP_NAME=_${JENNIC_CHIP}
	-DJENNIC_CHIP_FAMILY_NAME=_${JENNIC_CHIP_FAMILY}
	-DJENNIC_CHIP_FAMILY_${JENNIC_CHIP_FAMILY}
	-DJENNIC_CHIP_FAMILY=${JENNIC_CHIP_FAMILY}
	-DJN516x=5160
	-DDBG_ENABLE
	-DEMBEDDED
	-DPDM_NO_RTOS
    -DJENNIC_MAC_MiniMacShim
)


# Set up paths to JET, PDUMConfig, and ZPSConfig
find_package(Python3 COMPONENTS Interpreter)
# Prefer python scripts over binaries in SDK dir
if(Python3_Interpreter_FOUND AND NOT FORCE_SDK_BINARY_TOOLS)
    set(PDUM_CONFIG "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/PDUMConfig/PDUMConfig.py")
    set(ZPS_CONFIG "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/ZPSConfig/ZPSConfig.py")
    set(JET "${Python3_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/scripts/JET/jn_encryption_tool.py")
else()
    if(WIN32)
        set(PDUM_CONFIG "${SDK_PREFIX}/Tools/PDUMConfig/bin/PDUMConfig.exe")
        set(ZPS_CONFIG "${SDK_PREFIX}/Tools/ZPSConfig/bin/ZPSConfig.exe")
        set(JET "${SDK_PREFIX}/Tools/OTAUtils/JET.exe")
    elseif(LINUX)
        set(PDUM_CONFIG "${SDK_PREFIX}/Tools/PDUMConfig/linuxbin/PDUMConfig")
        set(ZPS_CONFIG "${SDK_PREFIX}/Tools/ZPSConfig/linuxbin/ZPSConfig")
        set(JET "${SDK_PREFIX}/Tools/OTALinuxUtils/JET")
    else()
        message(FATAL_ERROR "Unsupported platform for native Jennic SDK tools. Use Python version instead.")
    endif()
endif()

# Dump toolchain variables
function(dump_compiler_settings)
    message(STATUS "")
    message(STATUS "======================")
    message(STATUS "Toolchain paths:")
    message(STATUS "  TOOLCHAIN_BIN_DIR = ${TOOLCHAIN_BIN_DIR}")
    message(STATUS "  TOOLCHAIN_INC_DIR = ${TOOLCHAIN_INC_DIR}")
    message(STATUS "  TOOLCHAIN_LIB_DIR = ${TOOLCHAIN_LIB_DIR}")

    message(STATUS "  CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
    message(STATUS "  CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
    message(STATUS "  CMAKE_OBJCOPY = ${CMAKE_OBJCOPY}")
    message(STATUS "  CMAKE_OBJDUMP = ${CMAKE_OBJDUMP}")
    message(STATUS "  CMAKE_SIZE = ${CMAKE_SIZE}")
    message(STATUS "  CMAKE_DEBUGER = ${CMAKE_DEBUGER}")
    message(STATUS "  CMAKE_CPPFILT = ${CMAKE_CPPFILT}")

    message(STATUS "======================")
    message(STATUS "Compiler flags:")
    message(STATUS "  CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
    message(STATUS "  CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
    message(STATUS "  CMAKE_ASM_FLAGS = ${CMAKE_ASM_FLAGS}")
    message(STATUS "  CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
    message(STATUS "  CMAKE_MODULE_LINKER_FLAGS = ${CMAKE_MODULE_LINKER_FLAGS}")
    message(STATUS "  CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")

    message(STATUS "  CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "  CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "  CMAKE_ASM_FLAGS_DEBUG = ${CMAKE_ASM_FLAGS_DEBUG}")
    message(STATUS "  CMAKE_EXE_LINKER_FLAGS_DEBUG = ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    message(STATUS "  CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "  CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "  CMAKE_ASM_FLAGS_RELEASE = ${CMAKE_ASM_FLAGS_RELEASE}")
    message(STATUS "  CMAKE_EXE_LINKER_FLAGS_RELEASE = ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
    message(STATUS "======================")
    message(STATUS "SDK Tools:")
    message(STATUS "  PDUM_CONFIG = ${PDUM_COMFIG}")
    message(STATUS "  ZPS_CONFIG = ${ZPS_CONFIG}")
    message(STATUS "  JET = ${JET}")
    message(STATUS "======================")
    message(STATUS "")
endfunction()

# Every Zigbee application would need zps_gen.c and pdum_gen.c generated files
function(generate_zps_and_pdum_targets ZPSCFG_FILE)
    add_custom_command(
        OUTPUT
            zps_gen.c
            zps_gen.h
        COMMAND ${ZPS_CONFIG}
                -n ${BOARD}
                -f ${ZPSCFG_FILE}
                -o ${CMAKE_CURRENT_BINARY_DIR}
                -t ${JENNIC_CHIP}
                -l ${SDK_PREFIX}/Components/Library/libZPSNWK_${JENNIC_CHIP_FAMILY}.a
                -a ${SDK_PREFIX}/Components/Library/libZPSAPL_${JENNIC_CHIP_FAMILY}.a
                -c ${TOOLCHAIN_PREFIX}
        DEPENDS ${ZPSCFG_FILE}
    )

    add_custom_command(
        OUTPUT
            pdum_gen.c
            pdum_gen.h
            pdum_apdu.S
        COMMAND ${PDUM_CONFIG}
                -z ${BOARD}
                -f ${ZPSCFG_FILE}
                -o ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${ZPSCFG_FILE}
    )

    # Add path to generated headers globally (not using target_include_directories)
    # as generated files are also included from Zigbee sources
    include_directories(${CMAKE_CURRENT_BINARY_DIR})   
endfunction()

function(set_target_filename TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
        set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}" PARENT_SCOPE)
    else()
        set(FILENAME "${TARGET}" PARENT_SCOPE)
    endif()
endfunction()

function(add_hex_bin_targets TARGET)
    set_target_filename(${TARGET})

    add_custom_target(OUTPUT "${TARGET}.hex"
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex
    )

    add_custom_target("${TARGET}.bin"
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -j .version -j .bir -j .flashheader -j .vsr_table -j .vsr_handlers -j .rodata -j .text -j .data -j .bss -j .heap -j .stack -j .ro_mac_address -j .ro_ota_header -j .ro_se_lnkKey -j .pad -S -O binary ${FILENAME} ${FILENAME}.tmp.bin
        COMMAND ${JET} -m otamerge --embed_hdr -c ${FILENAME}.tmp.bin -v JN516x -n ${BUILD_NUMBER} -t ${FIRMWARE_FILE_TYPE} -u ${MANUFACTURER_ID} -j ${FIRMWARE_STRING} -o ${FILENAME}.bin
        COMMAND ${CMAKE_COMMAND} -E remove -f ${FILENAME}.tmp.bin
    )
endfunction()

function(add_ota_bin_target TARGET)
    set_target_filename(${TARGET})

    add_custom_target(${TARGET}.ota
        DEPENDS ${TARGET}.bin
        COMMAND ${JET} -m otamerge --ota -v JN516x -n ${BUILD_NUMBER} -t ${FIRMWARE_FILE_TYPE} -u ${MANUFACTURER_ID} -p 1 -c ${FILENAME}.bin -o ${FILENAME}.ota
    )
endfunction()

function(add_dump_target TARGET)
    set_target_filename(${TARGET})

    add_custom_target(${TARGET}.dump DEPENDS ${TARGET} COMMAND ${CMAKE_OBJDUMP} -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
endfunction()

function(print_size_of_targets TARGET)
    set_target_filename(${TARGET})
    
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})
endfunction()

function(add_flash_firmware_target TARGET)
    set_target_filename(${TARGET})

    add_custom_target(${TARGET}.flash DEPENDS ${TARGET}.bin COMMAND "C:\\NXP\\ProductionFlashProgrammer\\JN51xxProgrammer.exe" -V 0 -s ${FLASH_PORT} -f ${FILENAME}.bin)
endfunction()

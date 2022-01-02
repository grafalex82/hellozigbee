# Check mandatory parameters
if(NOT SDK_PREFIX)
     message(FATAL_ERROR "No SDK_PREFIX specified (it must point to the root of JN-SW-4170 SDK)")
endif()

if(NOT TOOLCHAIN_PREFIX)
     message(FATAL_ERROR "No TOOLCHAIN_PREFIX specified (it must point to the root of desired compiler bundle)")
endif()

if(NOT JENNIC_CHIP)
     message(FATAL_ERROR "No JENNIC_CHIP specified (it must reflect target chip name)")
endif()

# Load the toolchain file for a selected chip
get_filename_component(JENNIC_CMAKE_DIR ${CMAKE_CURRENT_LIST_FILE} DIRECTORY)
set(CMAKE_MODULE_PATH ${JENNIC_CMAKE_DIR} ${CMAKE_MODULE_PATH})

if(JENNIC_CHIP STREQUAL "JN5169")
     set(CMAKE_TOOLCHAIN_FILE ${JENNIC_CMAKE_DIR}/${JENNIC_CHIP}.cmake)     
     message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
else()
     message(FATAL_ERROR "Unsupported target chip - ${JENNIC_CHIP}")
endif()


# Dump toolchain variables
function(DUMP_COMPILER_SETTINGS)
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
     message(STATUS "")
endfunction()

function(ADD_HEX_BIN_TARGETS TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
      set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    else()
      set(FILENAME "${TARGET}")
    endif()
    add_custom_target(OUTPUT "${TARGET}.hex"
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -Oihex ${FILENAME} ${FILENAME}.hex
    )
    add_custom_target("${TARGET}.bin"
        DEPENDS ${TARGET}
        COMMAND ${CMAKE_OBJCOPY} -j .version -j .bir -j .flashheader -j .vsr_table -j .vsr_handlers -j .rodata -j .text -j .data -j .bss -j .heap -j .stack -j .ro_mac_address -j .ro_ota_header -j .pad -S -O binary ${FILENAME} ${FILENAME}.tmp.bin
        COMMAND "${SDK_PREFIX}\\Tools\\OTAUtils\\JET.exe" -m otamerge --embed_hdr -c ${FILENAME}.tmp.bin -v JN516x -n 1 -t 1 -u 0x1037 -j "HelloZigbee2021                 " -o ${FILENAME}.bin
    )
endfunction()

function(ADD_OTA_BIN_TARGETS TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
      set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    else()
      set(FILENAME "${TARGET}")
    endif()
    add_custom_target(${TARGET}.ota
        DEPENDS ${TARGET}.bin
	# HACK/TODO: setting file version to 2 (-n 2), so that OTA image is always newer than current version
        COMMAND "${SDK_PREFIX}\\Tools\\OTAUtils\\JET.exe" -m otamerge --embed_hdr -c ${FILENAME}.tmp.bin -v JN516x -n 2 -t 1 -u 0x1037 -j "HelloZigbee2021                 " -o ${FILENAME}.bin
        COMMAND "${SDK_PREFIX}\\Tools\\OTAUtils\\JET.exe" -m otamerge --ota -v JN516x -n 2 -t 1 -u 0x1037 -p 1 -c ${FILENAME}.bin -o ${FILENAME}.ota
    )
endfunction()

function(ADD_DUMP_TARGET TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
      set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    else()
      set(FILENAME "${TARGET}")
    endif()
    add_custom_target(${TARGET}.dump DEPENDS ${TARGET} COMMAND ${CMAKE_OBJDUMP} -x -D -S -s ${FILENAME} | ${CMAKE_CPPFILT} > ${FILENAME}.dump)
endfunction()

function(PRINT_SIZE_OF_TARGETS TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
      set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    else()
      set(FILENAME "${TARGET}")
    endif()
    add_custom_command(TARGET ${TARGET} POST_BUILD COMMAND ${CMAKE_SIZE} ${FILENAME})
endfunction()

function(FLASH_FIRMWARE_TARGET TARGET)
    if(RUNTIME_OUTPUT_DIRECTORY)
      set(FILENAME "${RUNTIME_OUTPUT_DIRECTORY}/${TARGET}")
    else()
      set(FILENAME "${TARGET}")
    endif()
    add_custom_target(${TARGET}.flash DEPENDS ${TARGET}.bin COMMAND "C:\\NXP\\ProductionFlashProgrammer\\JN51xxProgrammer.exe" -V 0 -s COM3 -f ${FILENAME}.bin)
endfunction()

# We will always be building Release in order to enable all needed optimizations
# Debug symbols will be still present in the ELF file
set(CMAKE_BUILD_TYPE Release)

# Prepare the toolchain for JN5169 microcontroller
set(JENNIC_CHIP JN5169)
include(cmake/JennicSDK.cmake)

# Main project settings
cmake_minimum_required(VERSION 3.8)
project(HelloZigbee)
enable_language(CXX)
enable_language(ASM)

# Select the board
set(SUPPORTED_BOARDS 
    "EBYTE_E75" # Custom 2-gang switch board based on EBYTE E75-2G4M20S1E module
    "QBKG11LM"  # Aqara Wall Switch (1-gang)
    "QBKG12LM"  # Aqara Wall Switch (2-gang)
    )
if(NOT DEFINED BOARD)
    set(BOARD "EBYTE_E75" CACHE STRING "Select the board to build for (${SUPPORTED_BOARDS})")
endif()

list(FIND SUPPORTED_BOARDS "${BOARD}" BOARD_INDEX)
if(BOARD_INDEX EQUAL -1)
    message(FATAL_ERROR "Unsupported BOARD specified. Supported options are: ${SUPPORTED_BOARDS}")
endif()

# Function that checks if a variable is provided, otherwise it will be set to a default value
function(set_build_param VAR_NAME DEFAULT_VALUE)
    if(NOT DEFINED ${VAR_NAME})
        set(${VAR_NAME} ${DEFAULT_VALUE} CACHE STRING "Default value for ${VAR_NAME}")
    endif()
endfunction()

# Set various build parameters
set_build_param(BUILD_NUMBER "1")
set_build_param(MANUFACTURER_ID "0x115F")
set_build_param(FIRMWARE_FILE_TYPE 257)
set_build_param(FIRMWARE_STRING "DR1175r1v1UNENCRYPTED00000JN5169")
string(TIMESTAMP TODAY "%Y-%m-%d")
set_build_param(BUILD_DATE "${TODAY}")
string(LENGTH BUILD_DATE BUILD_DATE_LEN)
set_build_param(VERSION_NUMBER "1.0")
set(VERSION_STR "${VERSION_NUMBER}.${BUILD_NUMBER}")
string(LENGTH "${VERSION_STR}" VERSION_STR_LEN)

set_build_param(FLASH_PORT "COM5")

#dump_compiler_settings()

add_subdirectory(src)

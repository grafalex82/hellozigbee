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

#dump_compiler_settings()

add_subdirectory(src)

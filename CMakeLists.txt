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

#dump_compiler_settings()

add_subdirectory(src)

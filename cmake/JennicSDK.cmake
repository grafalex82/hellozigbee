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

     MESSAGE(STATUS "======================")
     MESSAGE(STATUS "Compiler flags:")

     MESSAGE(STATUS "  CMAKE_C_FLAGS = ${CMAKE_C_FLAGS}")
     MESSAGE(STATUS "  CMAKE_CXX_FLAGS = ${CMAKE_CXX_FLAGS}")
     MESSAGE(STATUS "  CMAKE_ASM_FLAGS = ${CMAKE_ASM_FLAGS}")
     MESSAGE(STATUS "  CMAKE_EXE_LINKER_FLAGS = ${CMAKE_EXE_LINKER_FLAGS}")
     MESSAGE(STATUS "  CMAKE_MODULE_LINKER_FLAGS = ${CMAKE_MODULE_LINKER_FLAGS}")
     MESSAGE(STATUS "  CMAKE_SHARED_LINKER_FLAGS = ${CMAKE_SHARED_LINKER_FLAGS}")

     MESSAGE(STATUS "  CMAKE_C_FLAGS_DEBUG = ${CMAKE_C_FLAGS_DEBUG}")
     MESSAGE(STATUS "  CMAKE_CXX_FLAGS_DEBUG = ${CMAKE_CXX_FLAGS_DEBUG}")
     MESSAGE(STATUS "  CMAKE_ASM_FLAGS_DEBUG = ${CMAKE_ASM_FLAGS_DEBUG}")
     MESSAGE(STATUS "  CMAKE_EXE_LINKER_FLAGS_DEBUG = ${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
     MESSAGE(STATUS "  CMAKE_C_FLAGS_RELEASE = ${CMAKE_C_FLAGS_RELEASE}")
     MESSAGE(STATUS "  CMAKE_CXX_FLAGS_RELEASE = ${CMAKE_CXX_FLAGS_RELEASE}")
     MESSAGE(STATUS "  CMAKE_ASM_FLAGS_RELEASE = ${CMAKE_ASM_FLAGS_RELEASE}")
     MESSAGE(STATUS "  CMAKE_EXE_LINKER_FLAGS_RELEASE = ${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
     message(STATUS "======================")
     message(STATUS "")
endfunction()

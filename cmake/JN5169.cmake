set(CMAKE_SYSTEM_NAME Generic)

if(NOT TOOLCHAIN_PREFIX)
     message(STATUS "No TOOLCHAIN_PREFIX specified")
endif()


if(NOT IS_ABSOLUTE ${TOOLCHAIN_PREFIX})
    set(TOOLCHAIN_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/${TOOLCHAIN_PREFIX}")
endif()
get_filename_component(TOOLCHAIN_PREFIX ${TOOLCHAIN_PREFIX} ABSOLUTE)
message(STATUS "Using toolchain path: ${TOOLCHAIN_PREFIX}")


if(WIN32)
    set(TOOL_EXECUTABLE_SUFFIX ".exe")
ELSE()
    set(TOOL_EXECUTABLE_SUFFIX "")
endif()

set(TARGET_PREFIX "ba-elf")

set(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
set(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/include)
set(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/lib)


if(NOT CMAKE_C_COMPILER)
    set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-gcc${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "C compiler")
endif()

if(NOT CMAKE_CXX_COMPILER)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-g++${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "C++ compiler")
endif()

set(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-objcopy${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-objdump${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-size${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "size tool")
set(CMAKE_DEBUGER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-gdb${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "debuger")
set(CMAKE_CPPFILT ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-c++filt${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "C++filt")

set(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "c compiler flags debug")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
set(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")
set(CMAKE_C_FLAGS_RELEASE "-Os -g" CACHE INTERNAL "c compiler flags release")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g" CACHE INTERNAL "cxx compiler flags release")
set(CMAKE_ASM_FLAGS_RELEASE "-g" CACHE INTERNAL "asm compiler flags release")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "linker flags release")

set(CMAKE_C_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections" CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections -Dbool=int -fno-rtti -fno-exceptions -fno-use-cxa-atexit -fno-threadsafe-statics" CACHE INTERNAL "cxx compiler flags")
set(CMAKE_ASM_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections -Dbool=int -fno-use-cxa-atexit -fno-threadsafe-statics" CACHE INTERNAL "asm compiler flags")
set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-u_AppColdStart -Wl,-u_AppWarmStart -march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -Os -fshort-enums -nostartfiles -fno-rtti -fno-exceptions -fno-use-cxa-atexit -fno-threadsafe-statics -Wl,--gc-sections -Wl,--defsym=__stack_size=5000 -Wl,--defsym,__minimum_heap_size=2000 " CACHE INTERNAL "executable linker flags")
set(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "module linker flags")
set(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "shared linker flags")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX} ${EXTRA_FIND_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


SET(CMAKE_SYSTEM_NAME Generic)

IF(NOT TOOLCHAIN_PREFIX)
     MESSAGE(STATUS "No TOOLCHAIN_PREFIX specified")
ENDIF()

IF (WIN32)
    SET(TOOL_EXECUTABLE_SUFFIX ".exe")
ELSE()
    SET(TOOL_EXECUTABLE_SUFFIX "")
ENDIF()

SET(TARGET_PREFIX "ba-elf")

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/lib)

IF(NOT CMAKE_C_COMPILER)
	SET(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-gcc${TOOL_EXECUTABLE_SUFFIX})
ENDIF()

IF(NOT CMAKE_CXX_COMPILER)
	SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-g++${TOOL_EXECUTABLE_SUFFIX})
ENDIF()

SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-objcopy${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "objcopy tool")
SET(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-objdump${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "objdump tool")
SET(CMAKE_SIZE ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-size${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "size tool")
SET(CMAKE_DEBUGER ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-gdb${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "debuger")
SET(CMAKE_CPPFILT ${TOOLCHAIN_BIN_DIR}/${TARGET_PREFIX}-c++filt${TOOL_EXECUTABLE_SUFFIX} CACHE INTERNAL "C++filt")

SET(CMAKE_C_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "c compiler flags debug")
SET(CMAKE_CXX_FLAGS_DEBUG "-Og -g" CACHE INTERNAL "cxx compiler flags debug")
SET(CMAKE_ASM_FLAGS_DEBUG "-g" CACHE INTERNAL "asm compiler flags debug")
SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "" CACHE INTERNAL "linker flags debug")
SET(CMAKE_C_FLAGS_RELEASE "-Os -g" CACHE INTERNAL "c compiler flags release")
SET(CMAKE_CXX_FLAGS_RELEASE "-Os -g" CACHE INTERNAL "cxx compiler flags release")
SET(CMAKE_ASM_FLAGS_RELEASE "-g" CACHE INTERNAL "asm compiler flags release")
SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "" CACHE INTERNAL "linker flags release")

SET(CMAKE_C_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections" CACHE INTERNAL "c compiler flags")
SET(CMAKE_CXX_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections -fno-rtti -fno-exceptions -fno-use-cxa-atexit -fno-threadsafe-statics" CACHE INTERNAL "cxx compiler flags")
SET(CMAKE_ASM_FLAGS "-march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -fshort-enums -Wall -Wpacked -Wcast-align -fdata-sections -ffunction-sections -fno-use-cxa-atexit -fno-threadsafe-statics" CACHE INTERNAL "asm compiler flags")
SET(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections -Wl,-u_AppColdStart -Wl,-u_AppWarmStart -march=ba2 -mcpu=jn51xx -mredzone-size=4 -mbranch-cost=3 -fomit-frame-pointer -Os -fshort-enums -nostartfiles -fno-rtti -fno-exceptions -fno-use-cxa-atexit -fno-threadsafe-statics -Wl,--gc-sections -Wl,--defsym=__stack_size=5000 -Wl,--defsym,__minimum_heap_size=2000 " CACHE INTERNAL "executable linker flags")
SET(CMAKE_MODULE_LINKER_FLAGS "" CACHE INTERNAL "module linker flags")
SET(CMAKE_SHARED_LINKER_FLAGS "" CACHE INTERNAL "shared linker flags")

SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX} ${EXTRA_FIND_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


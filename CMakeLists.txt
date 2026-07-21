cmake_minimum_required(VERSION 3.5)

# option(BUILD_ARM "Build ARM" OFF)

# execute_process(
# COMMAND ${CMAKE_COMMAND} -E environment
# )
set(CURRENT_COMPILER "HOST")
set(CROSS_CXX_COMPILER "/opt/gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf/bin/arm-linux-gnueabihf-g++")
set(CROSS_CXX_FLAGS "-marm -mfloat-abi=hard -march=armv7-a -mfpu=neon -funsafe-math-optimizations -ftree-vectorize -fPIC")
set(ARM64_CROSS_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")
set(ARM64_CROSS_CXX_FLAGS "-funsafe-math-optimizations -ftree-vectorize -fPIC")

macro(use_host_compiler)
    if(${CURRENT_COMPILER} STREQUAL "CROSS")
        message("Switching to host compiler")

        # Save current cross flags
        set(CROSS_CXX_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "GCC flags for the cross compiler." FORCE)

        # Change compiler
        set(CMAKE_CXX_COMPILER ${HOST_CXX_COMPILER})
        set(CMAKE_CXX_FLAGS ${HOST_CXX_FLAGS})
        set(CURRENT_COMPILER "HOST" CACHE STRING "Which compiler we are using." FORCE)
    endif()
endmacro()

macro(use_cross_compiler)
    if(${CURRENT_COMPILER} STREQUAL "HOST")
        message("Switching to cross compiler")

        # Save current host flags
        set(HOST_CXX_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "GCC flags for the host compiler." FORCE)

        # Change compiler
        if(BUILD_KRIA)
            set(CMAKE_CXX_COMPILER ${ARM64_CROSS_CXX_COMPILER})
            set(CMAKE_CXX_FLAGS ${ARM64_CROSS_CXX_FLAGS})
        else()
            set(CMAKE_CXX_COMPILER ${CROSS_CXX_COMPILER})
            set(CMAKE_CXX_FLAGS ${CROSS_CXX_FLAGS})
        endif()

        set(CURRENT_COMPILER "CROSS" CACHE STRING "Which compiler we are using." FORCE)
    endif()
endmacro()

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
project(secda_tools)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules" ${CMAKE_MODULE_PATH})

if (NOT DEFINED SECDA_TOOLS_BUILD_SHARED)
    option(SECDA_TOOLS_BUILD_SHARED "Build secda_tools frontend libraries as shared objects" OFF)
endif()

if (NOT DEFINED SECDA_TOOLS_BUILD_TESTS)
    option(SECDA_TOOLS_BUILD_TESTS "Build secda_tools tests" ON)
endif()

if (NOT BUILD_ARM)
    find_package(SYSC REQUIRED)
endif()

add_subdirectory(secda_tools)
if (SECDA_TOOLS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

cmake_minimum_required(VERSION 3.16.0)

if(NOT MODS_BEING_BUILD STREQUAL "all")
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "arm")
endif()

# Start Project
project(expanded-creative-inventory)

# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake")

# Build
add_library(expanded-creative SHARED creative.cpp)
target_link_libraries(expanded-creative mods-headers reborn-patch symbols misc)

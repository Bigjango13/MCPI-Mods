cmake_minimum_required(VERSION 3.16.0)

if(NOT MODS_BEING_BUILD STREQUAL "all")
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "arm")
endif()

# Start Project
project(misc)

# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake")

# Build
add_library(hud SHARED hud.cpp)
target_link_libraries(hud mods-headers reborn-patch symbols misc)

add_library(block_logging SHARED block_logging.cpp)
target_link_libraries(block_logging mods-headers reborn-patch symbols misc)

add_library(pickblock SHARED pickblock.cpp)
target_link_libraries(pickblock mods-headers reborn-patch symbols misc)

add_library(shiftclick SHARED shiftclick.cpp)
target_link_libraries(shiftclick mods-headers reborn-patch symbols misc)

add_library(shiftitems SHARED shiftitems.cpp)
target_link_libraries(shiftitems mods-headers reborn-patch symbols misc)

add_library(extraslot SHARED extraslot.cpp)
target_link_libraries(extraslot mods-headers reborn-patch symbols misc)

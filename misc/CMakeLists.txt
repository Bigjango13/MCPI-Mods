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
target_link_libraries(hud mods reborn-patch symbols)

add_library(block_logging SHARED block_logging.cpp)
target_link_libraries(block_logging mods reborn-patch symbols)

add_library(pickblock SHARED pickblock.cpp)
target_link_libraries(pickblock mods reborn-patch symbols)

add_library(shiftclick SHARED shiftclick.cpp)
target_link_libraries(shiftclick mods reborn-patch symbols)

add_library(shiftitems SHARED shiftitems.cpp)
target_link_libraries(shiftitems mods reborn-patch symbols)

add_library(extraslot SHARED extraslot.cpp)
target_link_libraries(extraslot mods reborn-patch symbols)

add_library(invscrolling SHARED invscrolling.cpp)
target_link_libraries(invscrolling mods reborn-patch symbols)

add_library(dualwielding SHARED dualwielding.cpp)
target_link_libraries(dualwielding mods reborn-patch symbols)

add_library(sounds SHARED sounds.cpp)
target_link_libraries(sounds reborn-patch media-layer-core stdc++fs)

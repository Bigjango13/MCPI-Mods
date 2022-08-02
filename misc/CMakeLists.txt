cmake_minimum_required(VERSION 3.16.0)

if(NOT MODS_BEING_BUILD STREQUAL "all")
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
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

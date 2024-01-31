cmake_minimum_required(VERSION 3.16.0)

if(NOT MODS_BEING_BUILD STREQUAL "all")
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(CMAKE_SYSTEM_NAME "Linux")
    set(CMAKE_SYSTEM_PROCESSOR "arm")
endif()

# Start Project
project(mcpi-competition)

# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake")

# Build the jetpack
add_library(jetpack SHARED week-1/jetpack.cpp)
target_link_libraries(jetpack mods reborn-patch media-layer-core)

# Install custom jetpack textures
install(FILES week-1/images/jetpack_1.png DESTINATION "$ENV{HOME}/.minecraft-pi/overrides/images/armor")
install(FILES week-1/images/items.png DESTINATION "$ENV{HOME}/.minecraft-pi/overrides/images/gui")
install(FILES week-1/images/en_US.lang DESTINATION "$ENV{HOME}/.minecraft-pi/overrides/lang/")

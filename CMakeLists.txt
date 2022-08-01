cmake_minimum_required(VERSION 3.16.0)

# Build For ARM
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Start Project
project(mcpi-competition)

# Include SDK
include("$ENV{HOME}/.minecraft-pi/sdk/lib/minecraft-pi-reborn-client/sdk/sdk.cmake")

# Build the jetpack
add_library(jetpack SHARED week-1/jetpack.cpp)
target_link_libraries(jetpack mods-headers reborn-patch symbols misc mods-headers media-layer-core)

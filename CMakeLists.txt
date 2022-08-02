cmake_minimum_required(VERSION 3.16.0)

# Build For ARM
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

# Start Project
project(my-mcpi-mods)

set(MODS_BEING_BUILD "all")

# Build all the dirs
add_subdirectory(extended-creative-inventory)
add_subdirectory(mcpi-competition)
add_subdirectory(misc)

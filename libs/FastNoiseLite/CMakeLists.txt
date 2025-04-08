cmake_minimum_required(VERSION 3.10)
project(FastNoise)

# Create a header-only library target for FastNoiseLite
add_library(FastNoiseLite INTERFACE)
target_include_directories(FastNoiseLite INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
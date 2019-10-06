# Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# The following folder will be included
include_directories ("${PROJECT_SOURCE_DIR}/src")

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O3")

add_executable (e-adarp ${PROJECT_SOURCE_DIR}/src/main.cpp)

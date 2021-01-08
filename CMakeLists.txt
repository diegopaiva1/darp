# Specify the minimum version for CMake
cmake_minimum_required(VERSION 2.6)

# Project's name
project(darp)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

# Set the target name
set(TARGET ${PROJECT_NAME}.exe)

# Folder containing all the header (*.hpp) files
include_directories("include" "third-party")

# Manually add the sources (*.cpp) using the set command as follows
set(SOURCES src/main.cpp
            src/algorithms/reactive_grasp.cpp
            src/node.cpp
            src/request.cpp
            src/route.cpp
            src/run.cpp
            src/instance.cpp
            src/solution.cpp
            src/vehicle.cpp
            src/gnuplot.cpp)

# Set compiler options
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fopenmp -std=c++11 -O3")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# Generate the binary
add_executable(${TARGET} ${SOURCES})

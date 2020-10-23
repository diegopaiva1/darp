# Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (darp)

# Set the target name
set (TARGET ${PROJECT_NAME})

# Folder containing all the header (*.hpp) files
include_directories ("include")

# Manually add the sources (*.cpp) using the set command as follows
set (SOURCES src/main.cpp
             src/algorithms/reactive_grasp.cpp
             src/data-structures/node.cpp
             src/data-structures/request.cpp
             src/data-structures/route.cpp
             src/data-structures/run.cpp
             src/data-structures/instance.cpp
             src/data-structures/solution.cpp
             src/data-structures/vehicle.cpp
             src/utils/gnuplot.cpp
             src/utils/timer.cpp)

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fopenmp -std=c++1z -O3")

# Generate the binary
add_executable (${TARGET} ${SOURCES})

# Disable building tests and examples in libfort project
set (FORT_ENABLE_TESTING OFF CACHE INTERNAL "")

# Add path to each library root
add_subdirectory (third-party/libfort)
add_subdirectory (third-party/random)

# Add include path to compiler
target_link_libraries (${TARGET} PRIVATE fort
                                 effolkronium_random)

# Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# Folder containing all the header files
include_directories ("src")

# Manually add the sources using the set command as follows:
set (SOURCES src/main.cpp
             src/algorithms/Grasp.cpp
             src/data-structures/Node.cpp
             src/data-structures/Request.cpp
             src/data-structures/Route.cpp
             src/data-structures/Singleton.cpp
             src/data-structures/Solution.cpp
             src/data-structures/Vehicle.cpp
             src/utils/Prng.cpp
             src/utils/Timer.cpp)

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1z -Wall -O3")

add_executable (e-adarp ${SOURCES})

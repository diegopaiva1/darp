# Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# Folder containing all the header (*.hpp) files
include_directories ("include")

# Manually add the sources (*.cpp) using the set command as follows
set (SOURCES src/main.cpp
src/algorithms/ReactiveGrasp.cpp
src/data-structures/Node.cpp
src/data-structures/Request.cpp
src/data-structures/Route.cpp
src/data-structures/Singleton.cpp
src/data-structures/Solution.cpp
src/data-structures/Vehicle.cpp
src/utils/Prng.cpp
src/utils/Timer.cpp
src/utils/SolutionFileStorer.cpp
src/utils/Display.cpp
src/gnuplot/Gnuplot.cpp)

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++1z -Wall -O3")

add_executable (e-adarp ${SOURCES})

# Disable building tests and examples in libfort project
set (FORT_ENABLE_TESTING OFF CACHE INTERNAL "")

add_subdirectory (third-party/libfort)
target_link_libraries (e-adarp PRIVATE fort)

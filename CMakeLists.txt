# Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# Set the target name
set (TARGET ${PROJECT_NAME})

# Folder containing all the header (*.hpp) files
include_directories ("include")

# Manually add the sources (*.cpp) using the set command as follows
set (SOURCES src/main.cpp
             src/algorithms/ReactiveGrasp.cpp
             src/data-structures/Node.cpp
             src/data-structures/Request.cpp
             src/data-structures/Route.cpp
             src/data-structures/Run.cpp
             src/data-structures/Singleton.cpp
             src/data-structures/Solution.cpp
             src/data-structures/Vehicle.cpp
             src/utils/Display.cpp
             src/utils/Gnuplot.cpp
             src/utils/SolutionFileStorer.cpp
             src/utils/Timer.cpp)

# Set compiler options
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++1z -Wall -O3")

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

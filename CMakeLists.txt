 # Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# Store CPLEX's location to use during compile time
set (CPLEX_DIR "/opt/ibm/ILOG/CPLEX_Studio129")

# Add libraries tthat must be linked later
set (LDLIBS "-lilocplex -lconcert -lcplex -lm -lpthread -ldl")

# Set compiler options
set (CMAKE_CXX_FLAGS "-g -std=c++1z -Wall -O3 -I${CPLEX_DIR}/cplex/include -I${CPLEX_DIR}/concert/include -DIL_STD -L${CPLEX_DIR}/cplex/lib/x86-64_linux/static_pic -L${CPLEX_DIR}/concert/lib/x86-64_linux/static_pic")

# Folder containing all the header (*.hpp) files
include_directories ("src")

# Manually add the sources (*.cpp) using the set command as follows
set (SOURCES src/main.cpp
             src/algorithms/Grasp.cpp
             src/data-structures/Node.cpp
             src/data-structures/Request.cpp
             src/data-structures/Route.cpp
             src/data-structures/Singleton.cpp
             src/data-structures/Solution.cpp
             src/data-structures/Vehicle.cpp
             src/utils/Prng.cpp
             src/utils/Timer.cpp
             src/gnuplot/Gnuplot.cpp)

add_executable (e-adarp ${SOURCES})
target_link_libraries (e-adarp ${LDLIBS})

 # Specify the minimum version for CMake
cmake_minimum_required (VERSION 2.6)

# Project's name
project (e-adarp)

# Store CPLEX's location to use during compile time
set (CPLEX_DIR "/opt/ibm/ILOG/CPLEX_Studio129")

# Add libraries tthat must be linked later
set (LDLIBS "-lilocplex -lconcert -lcplex -lm -lpthread -ldl")

# Set compiler options
set (CMAKE_CXX_FLAGS "-g -std=c++11 -Wall -O3 -I${CPLEX_DIR}/cplex/include -I${CPLEX_DIR}/concert/include -DIL_STD -L${CPLEX_DIR}/cplex/lib/x86-64_linux/static_pic -L${CPLEX_DIR}/concert/lib/x86-64_linux/static_pic")

# The following folder will be included
include_directories ("${PROJECT_SOURCE_DIR}/src")

add_executable (e-adarp ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_link_libraries (e-adarp ${LDLIBS})

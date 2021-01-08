# Dial-a-ride problem (DARP)

## Requirements

*  *nix operating system (Ubuntu, Fedora, macOS, ...)
*  GCC compiler >= 4.8.1
*  [OpenMP library](https://www.openmp.org/)

## Building with CMake

```shell
mkdir build && cd build 
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

## Building from command line

```shell
mkdir build && cd build 
g++ -Wall -fopenmp -std=c++11 -O3 -o darp.exe -I ../include -I ../third-party $(find ../src -type f -name "*.cpp")
```

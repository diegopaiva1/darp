# Dial-a-ride problem (DARP)

## Requirements

*  *nix operating system (Ubuntu, Fedora, macOS, ...)
*  GCC compiler >= 4.8.1
*  [OpenMP library](https://www.openmp.org/)

If you wish to generate plots, [gnuplot](http://www.gnuplot.info/) should be installed in your system. 

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

## Example usage

```shell
./darp.exe ../data/instances/R1a.txt 5 8 out.json
```

This command will run the program five times on the instance R1a, using 8 threads (if GRASP). Information and statistics regarding all runs are written in [JSON](https://en.wikipedia.org/wiki/JSON) notation to file `out.json`.

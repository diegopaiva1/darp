# Heuristic Approaches to the Dial-a-ride Problem (DARP)

The code in this repository was developed as the foundation for my research to obtain a BSc in Computer Science.

[Click here](https://github.com/diegopaiva1/darp/blob/master/papers/Heuristic%20Approaches%20to%20the%20Dial-a-Ride%20Problem.pdf) to read the paper containing details and results of the algorithms.

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

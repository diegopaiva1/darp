/**
 * @file    Prng.hpp
 * @author  Diego Paiva
 * @date    13/02/2019
 */

#include "Prng.hpp"
#include <iostream>

std::tuple<int, int> Prng::generateInteger(int min, int max)
{
  // Use random_device to generate a seed for Mersenne twister engine
  std::random_device seeder;
  int seed = seeder();

  // Use Mersenne twister engine to generate pseudo-random numbers
  std::mt19937 engine(seed);

  // "Filter" MT engine's output to generate pseudo-random integer values, uniformly distributedon the interval [min, max]
  std::uniform_int_distribution<int> generate(min, max);

  // Generate pseudo-random integer
  return std::make_tuple(generate(engine), seed);
}

std::tuple<double, int> Prng::generateDouble(double min, double max)
{
  // Use random_device to generate a seed for Mersenne twister engine
  std::random_device seeder;
  int seed = seeder();

  // Use Mersenne twister engine to generate pseudo-random numbers
  std::mt19937 engine(seed);

  // "Filter" MT engine's output to generate pseudo-random double values, uniformly distributedon the interval [min, max)
  std::uniform_real_distribution<double> generate(min, max);

  // Generate pseudo-random double
  return std::make_tuple(generate(engine), seed);
}

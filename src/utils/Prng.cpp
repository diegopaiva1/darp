/**
 * @file    Prng.hpp
 * @author  Diego Paiva
 * @date    13/02/2019
 */

#include "Prng.hpp"

int Prng::generateIntegerInRange(int min, int max)
{
  std::random_device seeder;
  std::mt19937 engine(seeder());
  std::uniform_int_distribution<int> generate(min, max);
  return (int) generate(engine);
}

double Prng::generateDoubleInRange(double min, double max)
{
  std::random_device seeder;
  std::mt19937 engine(seeder());
  std::uniform_real_distribution<double> generate(min, max);
  return generate(engine);
}

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

float Prng::generateFloatInRange(float min, float max)
{
  std::random_device seeder;
  std::mt19937 engine(seeder());
  std::uniform_real_distribution<float> generate(min, max);
  return (float) generate(engine);
}

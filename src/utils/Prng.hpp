/**
 * @file    Prng.hpp
 * @author  Diego Paiva e Silva
 * @date    13/02/2019
 *
 * 'Prng' é o acrônimo para 'Pseudo Random Number Generator'.
 * Esta classe utiliza o algoritmo Mersenne Twister para gerar números pseudo-aleatórios inteiros ou reais
 * dentro de um intervalo especificado.
 */

#ifndef PRNG_H_INCLUDED
#define PRNG_H_INCLUDED

#include <random>

class Prng
{
public:
  // Intervalo [min, max] é fechado em 'min' e 'max'
  static int generateIntegerInRange(int min, int max)
  {
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_int_distribution<int> generate(min, max);
    return (int) generate(engine);
  }

  // Intervalo [min, max) é fechado em 'min' e aberto em 'max'
  static float generateFloatInRange(float min, float max)
  {
    std::random_device seeder;
    std::mt19937 engine(seeder());
    std::uniform_real_distribution<float> generate(min, max);
    return (float) generate(engine);
  }
};

#endif // PRNG_H_INCLUDED

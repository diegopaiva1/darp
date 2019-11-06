/**
 * @file    Prng.hpp
 * @author  Diego Paiva
 * @date    13/02/2019
 *
 * 'Prng' é o acrônimo para 'Pseudo Random Number Generator'.
 * Esta classe utiliza o algoritmo Mersenne Twister para gerar números pseudo-aleatórios inteiros ou reais
 * dentro de um intervalo especificado.
 */

#ifndef PRNG_HPP_INCLUDED
#define PRNG_HPP_INCLUDED

#include <random>

class Prng
{
public:
  // Intervalo [min, max] é fechado em 'min' e 'max'
  static int generateIntegerInRange(int min, int max);

  // Intervalo [min, max) é fechado em 'min' e aberto em 'max'
  static float generateFloatInRange(float min, float max);
};

#endif // PRNG_HPP_INCLUDED

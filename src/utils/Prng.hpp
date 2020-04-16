/**
 * @file    Prng.hpp
 * @author  Diego Paiva
 * @date    13/02/2019
 *
 * This is a pseudo random number generator utility based on the Mersenne Twister algorithm.
 */

#ifndef PRNG_HPP_INCLUDED
#define PRNG_HPP_INCLUDED

#include <random>

class Prng
{
public:
 /**
  * @brief Use Mersenne Twister algorithm to generate a random integer in interval [min, max].
  *
  * @param min Lower bound.
  * @param max Upper bound.
  *
  * @return A pair containing the (0) random integer and (1) the seed.
  */
  static std::pair<int, int> generateInteger(int min, int max);

 /**
  * @brief Use Mersenne Twister algorithm to generate a random double in interval [min, max).
  *
  * @param min Lower bound.
  * @param max Upper bound.
  *
  * @return A pair containing the (0) random double and (1) the seed.
  */
  static std::pair<double, int> generateDouble(double min, double max);
};

#endif // PRNG_HPP_INCLUDED

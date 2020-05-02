/**
 * @file    Run.hpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/Run.hpp"

Run::Run(Solution solution, double elapsedMinutes, unsigned int seed, int optimalIteration)
{
  this->solution         = solution;
  this->elapsedMinutes   = elapsedMinutes;
  this->seed             = seed;
  this->optimalIteration = optimalIteration;
}

Run::~Run()
{
  // Empty destructor
}

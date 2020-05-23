/**
 * @file    Run.hpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/Run.hpp"

Run::Run(Solution best, double elapsedMinutes, unsigned int seed, int bestIteration, double bestAlpha)
{
  this->best           = best;
  this->elapsedMinutes = elapsedMinutes;
  this->seed           = seed;
  this->bestIteration  = bestIteration;
  this->bestAlpha      = bestAlpha;
}

Run::~Run()
{
  // Empty destructor
}

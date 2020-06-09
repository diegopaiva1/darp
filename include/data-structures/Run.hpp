/**
 * @file   Run.hpp
 * @author Diego Paiva
 * @date   02/05/2020
 *
 * Data structure to store relevant info about an algorithm's run.
 */

#ifndef RUN_H_INCLUDED
#define RUN_H_INCLUDED

#include "Solution.hpp"

#include <map>

class Run
{
public:
  Solution bestInit;
  Solution best;
  double elapsedMinutes;
  unsigned int seed;
  int bestIteration;
  double bestAlpha;
  std::map<double, double> probDistribution;

 /**
  * @brief Default constructor.
  *
  * @param best             Best solution found in the run.
  * @param elapsedMinutes   Time in minutes spent in the run to achieve the Solution.
  * @param seed             Seed used in the Random engine.
  * @param bestIteration    Iteration in which the best solution was found.
  * @param bestAlpha        Alpha that led to the best solution.
  * @param probDistribution Alphas probability Distribution.
  */
  Run(Solution bestInit, Solution best, double elapsedMinutes, unsigned int seed, int bestIteration,
      double bestAlpha, std::map<double, double> probDistribution);

 /**
  * @brief Default destructor.
  */
  ~Run();

 /**
  * @brief Persist run info to file.
  *
  * @param fileName File Name.
  */
  void persist(std::string fileName);
};

#endif // RUN_H_INCLUDED

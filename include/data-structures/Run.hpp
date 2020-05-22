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

class Run
{
public:
  Solution     solution;
  double       elapsedMinutes;
  unsigned int seed;
  int          bestIteration;

 /**
  * @brief Default constructor.
  *
  * @param solution         Solution.
  * @param elapsedMinutes   Time in minutes spent in the run to achieve the Solution.
  * @param seed             Seed used in the Random engine.
  * @param bestIteration    Iteration in which the best solution was found.
  */
  Run(Solution solution, double elapsedMinutes, unsigned int seed, int bestIteration);

 /**
  * @brief Default destructor.
  */
  ~Run();
};

#endif // RUN_H_INCLUDED

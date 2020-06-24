/**
 * @file   Run.hpp
 * @author Diego Paiva
 * @date   02/05/2020
 *
 * Data structure to store relevant data from the run of the Reactive GRASP.
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
  double bestAlpha;
  int bestIteration;
  unsigned int seed;
  std::map<double, double> probDistribution;

 /**
  * @brief Default constructor.
  */
  Run();

 /**
  * @brief Default destructor.
  */
  ~Run();

 /**
  * @brief Persist run data to file.
  *
  * @param fileName File name.
  */
  void persist(std::string fileName);
};

#endif // RUN_H_INCLUDED

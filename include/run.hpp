/**
 * @file   run.hpp
 * @author Diego Paiva
 * @date   02/05/2020
 *
 * Data structure to store relevant data from the run of the Reactive GRASP.
 */

#ifndef RUN_H_INCLUDED
#define RUN_H_INCLUDED

#include "solution.hpp"

#include <map>

class Run
{
public:
  Solution best_init;
  Solution best;
  double elapsed_seconds;
  std::vector<unsigned int> seeds;

 /**
  * Default constructor.
  */
  Run() {};

 /**
  * Default destructor.
  */
  ~Run() {};
};

#endif // RUN_H_INCLUDED

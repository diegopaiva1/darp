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
#include <memory>

class Run
{
public:
  Solution best_init;
  Solution best;
  double elapsed_seconds;
  std::map<double, double> alphas_prob_distribution;

 /**
  * Default constructor.
  */
  Run() {};

 /**
  * Default destructor.
  */
  ~Run() {};

 /**
  * Persist run data to file.
  *
  * @param file_name File name.
  */
  void persist(std::string file_name);

 /**
  * Convert Run to string representation.
  */
  std::string to_string();
};

#endif // RUN_H_INCLUDED

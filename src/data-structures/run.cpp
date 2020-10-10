/**
 * @file    run.cpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/run.hpp"
#include "data-structures/instance.hpp"

#include <fstream>
#include <iomanip>
#include <thread>

void Run::persist(std::string file_name)
{
  std::fstream file(file_name, std::fstream::in | std::fstream::out | std::fstream::app);
  file.seekg(0, std::ios::end);

  // Empty?
  if (file.tellg() == 0)
    // Start writing header first
    file << "Best;Init;CPU (min);Best iteration;Best alpha;Best seed;Feasible\n";

  // Write the actual data
  file << std::fixed << std::setprecision(2)
       << best.obj_func_value() << ';' << best_init.obj_func_value() << ';' << elapsed_minutes << ';'
       << best_iteration << ';' << best_alpha << ';' << seed << ';' << best.feasible() << "\n";
}
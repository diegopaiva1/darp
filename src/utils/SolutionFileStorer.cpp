/**
 * @file   SolutionFileStorer.cpp
 * @author Diego Paiva
 * @date   07/05/2020
 */

#include "utils/SolutionFileStorer.hpp"

#include <iomanip> // std::fixed, std::setprecision

void SolutionFileStorer::saveRun(std::string fileName, Run run)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  if (empty(file))
    // Start writing header first
    file << "TT;ERT;Cost;CPU (min);Vehicles;Opt. iteration;Seed\n";

  // Write the actual data
  file << std::fixed << std::setprecision(2)
       << run.solution.travelTime << ';' << run.solution.excessRideTime << ';' << run.solution.cost    << ';'
       << run.elapsedMinutes      << ';' << run.solution.routes.size()  << ';' << run.optimalIteration << ';'
       << run.seed << "\n";
}

bool SolutionFileStorer::empty(std::fstream &file)
{
  // Get length of file
  file.seekg(0, std::ios::end);
  int length = file.tellg();

  return length == 0;
}

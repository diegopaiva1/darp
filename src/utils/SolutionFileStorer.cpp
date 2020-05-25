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
    file << "TT;ERT;Cost;CPU (min);Vehicles;Best iteration;Best alpha; Seed\n";

  // Write the actual data
  file << std::fixed << std::setprecision(2)
       << run.best.travelTime() << ';' << run.best.excessRideTime() << ';' << run.best.obj()     << ';'
       << run.elapsedMinutes    << ';' << run.best.routes.size()    << ';' << run.bestIteration  << ';'
       << run.bestAlpha         << ';' << run.seed << "\n";
}

bool SolutionFileStorer::empty(std::fstream &file)
{
  // Get length of file
  file.seekg(0, std::ios::end);
  int length = file.tellg();

  return length == 0;
}

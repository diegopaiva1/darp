/**
 * @file    Run.hpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/Run.hpp"

#include <fstream>
#include <iomanip>

Run::Run()
{
  // Empty constructor
}

Run::Run(Solution bestInit, Solution best, double elapsedMinutes, unsigned int bestSeed, int bestIteration,
         double bestAlpha, std::map<double, double> probDistribution)
{
  this->bestInit = bestInit;
  this->best = best;
  this->elapsedMinutes = elapsedMinutes;
  this->bestSeed = bestSeed;
  this->bestIteration = bestIteration;
  this->bestAlpha = bestAlpha;
  this->probDistribution = probDistribution;
}

Run::~Run()
{
  // Empty destructor
}

void Run::persist(std::string fileName)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  file.seekg(0, std::ios::end);

  // Empty?
  if (file.tellg() == 0)
    // Start writing header first
    file << "TT;ERT;Best;Init;CPU (min);Best iteration;Best alpha;Best seed;Feasible\n";

  // Write the actual data
  file << std::fixed << std::setprecision(2)
       << best.travelTime() << ';' << best.excessRideTime() << ';' << best.obj()      << ';'
       << bestInit.obj()    << ';' << elapsedMinutes        << ';' << bestIteration   << ';'
       << bestAlpha         << ';' << bestSeed              << ';' << best.feasible() << "\n";
}

/**
 * @file    Run.hpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/Run.hpp"

#include <fstream>
#include <iomanip>

Run::Run(double initialObj, Solution best, double elapsedMinutes, unsigned int seed, int bestIteration,
         double bestAlpha, std::map<double, double> probDistribution)
{
  this->initialObj       = initialObj;
  this->best             = best;
  this->elapsedMinutes   = elapsedMinutes;
  this->seed             = seed;
  this->bestIteration    = bestIteration;
  this->bestAlpha        = bestAlpha;
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
    file << "TT;ERT;Cost;CPU (min);Vehicles;Best iteration;Best alpha;Seed;Feasible\n";

  // Write the actual data
  file << std::fixed << std::setprecision(2)
       << best.travelTime() << ';' << best.excessRideTime() << ';' << best.obj()      << ';'
       << elapsedMinutes    << ';' << best.routes.size()    << ';' << bestIteration   << ';'
       << bestAlpha         << ';' << seed                  << ';' << best.feasible() << "\n";
}

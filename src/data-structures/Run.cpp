/**
 * @file    Run.hpp
 * @author  Diego Paiva
 * @date    02/05/2020
 */

#include "data-structures/Run.hpp"
#include "data-structures/Singleton.hpp"

#include <fstream>
#include <iomanip>
#include <thread>

Run::Run()
{
  // Empty constructor
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
       << best.travelTime() << ';' << best.excessRideTime() << ';' << best.objFuncValue() << ';'
       << bestInit.objFuncValue() << ';' << elapsedMinutes << ';' << bestIteration << ';'
       << bestAlpha << ';' << seed << ';' << best.feasible() << "\n";
}

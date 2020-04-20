/**
 * @file   SolutionFileStorer.cpp
 * @author Diego Paiva
 * @date   07/05/2020
 *
 * <Class description here>
 */

#include "utils/SolutionFileStorer.hpp"

#include <iomanip>

void SolutionFileStorer::storeSolution(std::string fileName, Solution s, double elapsedTime)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  if (isEmpty(file))
    // In case we have a empty file, start writing the header
    file << "Veiculos;TT;ERT;Custo;CPU (min)\n";

  // Write the actual data
  file << s.routes.size() << ';' << std::fixed << std::setprecision(2) << s.travelTime
                          << ';' << s.excessRideTime << ';' << s.cost << ';' << elapsedTime << "\n";
}

bool SolutionFileStorer::isEmpty(std::fstream &file)
{
  // Get length of file
  file.seekg(0, std::ios::end);
  int length = file.tellg();

  return length == 0;
}

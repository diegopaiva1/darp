/**
 * @file   SolutionFileStorer.cpp
 * @author Diego Paiva
 * @date   07/05/2020
 *
 * <Class description here>
 */

#include "utils/SolutionFileStorer.hpp"

#include <iomanip> // std::fixed, std::setprecision

void SolutionFileStorer::storeSolution(std::string fileName, Solution s, double elapsedTime, uint seed)
{
  std::fstream file(fileName, std::fstream::in | std::fstream::out | std::fstream::app);

  if (isEmpty(file))
    // Start writing header first
    file << "Veiculos;TT;ERT;Custo;CPU (min);Seed\n";

  // Write the actual data
  file << s.routes.size() << ';' << std::fixed << std::setprecision(2) << s.travelTime
                          << ';' << s.excessRideTime << ';' << s.cost << ';' << elapsedTime
                          << ';' << seed << "\n";
}

bool SolutionFileStorer::isEmpty(std::fstream &file)
{
  // Get length of file
  file.seekg(0, std::ios::end);
  int length = file.tellg();

  return length == 0;
}

/**
 * @file   SolutionFileStorer.hpp
 * @author Diego Paiva
 * @date   07/05/2020
 *
 * <Class description here>
 */

#ifndef SOLUTIONFILESTORER_H_INCLUDED
#define SOLUTIONFILESTORER_H_INCLUDED

#include "data-structures/Solution.hpp"

class SolutionFileStorer
{
public:
  static void storeSolution(std::string fileName, Solution s, double elapsedTime);

private:
  static bool isEmpty(std::fstream &file);
};

#endif // SOLUTIONFILESTORER_H_INCLUDED

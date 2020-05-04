/**
 * @file   SolutionFileStorer.hpp
 * @author Diego Paiva
 * @date   07/05/2020
 *
 * Utility to provide an easy way to save data from a run into a file.
 */

#ifndef SOLUTIONFILESTORER_HPP_INCLUDED
#define SOLUTIONFILESTORER_HPP_INCLUDED

#include "data-structures/Run.hpp"

#include <fstream>
#include <string>

class SolutionFileStorer
{
public:
 /**
  * @brief Save a run into a file.
  *
  * @param fileName Output file name.
  * @param run      A Run object.
  */
  static void saveRun(std::string fileName, Run run);

private:
 /**
  * @brief Check if file stream is empty.
  *
  * @param file File stream object.
  * @return     True if file is empty.
  */
  static bool empty(std::fstream &file);
};

#endif // SOLUTIONFILESTORER_HPP_INCLUDED

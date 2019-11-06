/**
 * @file    Gnuplot.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#ifndef GNUPLOT_HPP_INCLUDED
#define GNUPLOT_HPP_INCLUDED

#include <fstream>
#include <iostream>

#include "../data-structures/Singleton.hpp"
#include "../data-structures/Solution.hpp"

class Gnuplot
{
public:
 /**
   * @brief Plot a solution.
   *
   * @param s A solution to be ploted.
   */
  static void plotSolution(Solution s);
};

#endif // GNUPLOT_HPP_INCLUDED

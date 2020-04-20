/**
 * @file    Gnuplot.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#ifndef GNUPLOT_HPP_INCLUDED
#define GNUPLOT_HPP_INCLUDED

#include <fstream>
#include <iostream>

#include "data-structures/Singleton.hpp"
#include "data-structures/Solution.hpp"

class Gnuplot
{
public:
/**
  * @brief Plot a solution, which consists of a set of routes and a schedule for every route.
  *
  * @param s A solution to be ploted.
  */
  static void plotSolution(Solution s);

protected:
  static inline const std::string graphScript       = "../src/gnuplot/scripts/graph.gp";
  static inline const std::string scheduleScript    = "../src/gnuplot/scripts/schedule.gp";
  static inline const std::string destinationFolder = "../tmp/gnuplot/";

 /**
  * @brief Plot the graph containing all nodes and the links between them.
  *
  * @param routes A set of routes.
  * @param cost   Sum of all routes' cost.
  */
  static void plotGraph(std::vector<Route> routes, double cost);

 /**
  * @brief Plot the schedule of a given route.
  *
  * @param r A route.
  */
  static void plotSchedule(Route r);
};

#endif // GNUPLOT_HPP_INCLUDED

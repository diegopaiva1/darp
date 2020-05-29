/**
 * @file    Gnuplot.hpp
 * @author  Diego Paiva
 * @date    23/10/2019
 */

#ifndef GNUPLOT_HPP_INCLUDED
#define GNUPLOT_HPP_INCLUDED

#include "data-structures/Run.hpp"

#include <map>
#include <string>

class Gnuplot
{
public:
/**
  * @brief Plot a solution, which consists of a set of routes and a schedule for every route.
  *
  * @param s A solution to be ploted.
  */
  static void plotRun(Run run);

/**
  * @brief Get directory where plots will be stored.
  *
  * @return Directory string.
  */
  static const std::string getDestinationDir();

protected:
  static inline const std::string graphScript    = "../extras/scripts/gnuplot/graph.gp";
  static inline const std::string scheduleScript = "../extras/scripts/gnuplot/schedule.gp";
  static inline const std::string alphaScript    = "../extras/scripts/gnuplot/alphas.gp";
  static inline const std::string destinationDir = "../data/plots/";

 /**
  * @brief Plot the graph containing all nodes and the links between them.
  *
  * @param routes A set of routes.
  * @param cost   Sum of all routes' cost.
  * @param output Output file name.
  */
  static void plotGraph(std::vector<Route> routes, double cost, std::string output);

 /**
  * @brief Plot the schedule of a given route.
  *
  * @param r      A route.
  * @param output Output file name.
  */
  static void plotSchedule(Route r, std::string output);

  static void plotAlphasProbabilityDistribution(std::map<double, double> alphaWithProbs, std::string output);
};

#endif // GNUPLOT_HPP_INCLUDED

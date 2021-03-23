/**
 * @file   gnuplot.hpp
 * @author Diego Paiva
 * @date   23/10/2019
 */

#ifndef GNUPLOT_HPP_INCLUDED
#define GNUPLOT_HPP_INCLUDED

#include "run.hpp"

namespace gnuplot
{
 /**
  * Generate plots of a Run object. Plots are:
  *
  *   1. Initial solution graph;
  *   2. Best solution graph;
  *   3. Schedule for every route in best solution;
  *
  * @param run Run object.
  * @param dir Directory where generated plots will be stored.
  */
  void plot_run(Run run, std::string dir);

 /**
  * Plot convergence analysis of runs.
  *
  * @param runs   Hash map from algorithm name to runs.
  * @param output Output file name.
  */
  void plot_convergence_analysis(std::unordered_map<std::string, std::vector<Run>> runs, std::string output);

 /**
  * Produce time-to-target plot (tttplot).
  *
  * @param runs   Hash map from algorithm name to runs.
  * @param target Target value.
  * @param output Output file name.
  */
  void tttplot(std::unordered_map<std::string, std::vector<Run>> runs, double target, std::string output);

  namespace details
  {
   /**
    * Plot the graph of a given Solution.
    *
    * @param s      Solution.
    * @param output Output file name.
    */
    void plot_solution_graph(Solution s, std::string output);

   /**
    * Plot the schedule of a given route.
    *
    * @param r      A route.
    * @param output Output file name.
    */
    void plot_schedule(Route r, std::string output);

   /**
    * Open a pipe to send commands to gnuplot.
    *
    * @param args Command-line arguments to gnuplot.
    */
    void call_gnuplot(std::vector<std::string> args);
  }
}

#endif // GNUPLOT_HPP_INCLUDED

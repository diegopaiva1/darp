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
  *   4. Alphas probability distribution.
  *
  * @param run Run object.
  * @param dir Directory where generated plots will be stored.
  */
  void plot_run(Run run, std::string dir);

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
    * Plot alphas probability distribution.
    *
    * @param alphas_probs Map containing the alpha values and their probabilities.
    * @param output       Output file name.
    */
    void plot_alphas_probs(std::map<double, double> alphas_probs, std::string output);

   /**
    * Open a pipe to send commands to gnuplot.
    *
    * @param args Command-line arguments to gnuplot.
    */
    void call_gnuplot(std::vector<std::string> args);
  }
}

#endif // GNUPLOT_HPP_INCLUDED

/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "data-structures/instance.hpp"
#include "algorithms/reactive_grasp.hpp"
#include "utils/gnuplot.hpp"
#include "utils/display.hpp"

#include <iostream> // std::cerr

int main(const int argc, const char* argv[])
{
  const int min_args = 1;
  const int max_args = 2;
  const int args_given = argc - 1;

  if (args_given < min_args || args_given > max_args) {
    std::cerr << "Usage: " << argv[0] << " <instance> <save_file (optional)>\n";
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  Run run = ReactiveGrasp::solve(3000, 300, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});

  display::show_run(run);
  gnuplot::plot_run(run, "../data/plots/");

  if (argv[2])
    run.persist(argv[2]);

  return EXIT_SUCCESS;
}

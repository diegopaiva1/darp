/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "data-structures/instance.hpp"
#include "algorithms/reactive_grasp.hpp"
#include "utils/gnuplot.hpp"

#include <iostream> // std::cerr, std::cout

int main(const int argc, const char* argv[])
{
  const int min_args = 2, max_args = 3, args_given = argc - 1;

  if (args_given < min_args || args_given > max_args) {
    std::cerr << "Usage: " << argv[0] << " <instance> <threads> <save_file (optional)>\n";
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  Run run = reactive_grasp::solve(4096, 128, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0}, std::stoi(argv[2]));

  std::cout << run.to_string();
  gnuplot::plot_run(run, "../data/plots/");

  if (argv[3])
    run.persist(argv[3]);

  return EXIT_SUCCESS;
}

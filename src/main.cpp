/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "data-structures/instance.hpp"
#include "algorithms/reactive_grasp.hpp"
#include "utils/Gnuplot.hpp"
#include "utils/Display.hpp"

#include <iostream>

int main(const int argc, const char *argv[])
{
  const int min_args = 1;
  const int args_given = argc - 1;

  if (args_given < min_args) {
    std::cout << "ERROR: expected at least " << min_args << " arg(s); " << args_given << " given" << '\n';
    return EXIT_FAILURE;
  }

  inst.init(argv[1]);

  Run run = ReactiveGrasp::solve(3000, 300, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});

  Gnuplot::plotRun(run);
  Display::printRun(run);

  if (argv[2])
    run.persist(argv[2]);

  return EXIT_SUCCESS;
}

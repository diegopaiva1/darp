/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include "data-structures/Singleton.hpp"
#include "algorithms/ReactiveGrasp.hpp"
#include "utils/Gnuplot.hpp"
#include "utils/Display.hpp"

int main(const int argc, char const *argv[])
{
  const int minArgsAmount = 1;
  const int argsGiven     = argc - 1;

  if (argsGiven < minArgsAmount) {
    printf("Error: expected at least %d argument(s) - %d given\n", minArgsAmount, argsGiven);
    return EXIT_FAILURE;
  }

  inst->init(argv[1]);

  Run run = ReactiveGrasp::solve(100, 300, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});

  Gnuplot::plotRun(run);

  Display::printRun(run);

  if (argv[2])
    run.persist(argv[2]);

  return EXIT_SUCCESS;
}

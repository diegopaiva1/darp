/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"
#include "gnuplot/Gnuplot.hpp"
#include "utils/Timer.hpp"

#define MIN_ARGS_AMOUNT 1

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  Timer timer;
  Solution solution = Grasp::solve(1000, 100, {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0});
  double elapsed = timer.elapsedInSeconds();

  for (Route route : solution.routes) {
    route.printPath();
    printf("\n");
    route.printSchedule();
    printf("\n");
  }

  if (solution.isFeasible())
    printf("Viável\nCusto = %.2f\nt = %.2fs\n", solution.cost, elapsed);
  else
    printf("Inviável\nCusto = %.2f [load = %d, timeWindow = %.2f, maxRideTime = %.2f]\nt = %.2fs\n",
            solution.cost, solution.loadViolation, solution.timeWindowViolation, solution.maxRideTimeViolation, elapsed);

  Gnuplot::plotSolution(solution);

  return EXIT_SUCCESS;
}

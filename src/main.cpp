/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"
#include "gnuplot/Gnuplot.hpp"

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

  Solution solution = Grasp::solve(10000, 1000, {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50});

  for (Route *route : solution.routes) {
    route->printPath();
    printf("\n");
    route->printSchedule();
    printf("\n");
  }

  if (solution.isFeasible())
    printf("Viável - Custo = %.2f\n", solution.cost);
  else
    printf("Inviável - Custo = %.2f [load = %d, timeWindow = %.2f, maxRideTime = %.2f]\n",
            solution.cost, solution.loadViolation, solution.timeWindowViolation, solution.maxRideTimeViolation);

  Gnuplot::plotSolution(solution);

  return EXIT_SUCCESS;
}

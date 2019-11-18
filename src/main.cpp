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

    for (int i = 0; i < route.path.size(); i++) {
      if (route.serviceBeginningTimes[i] > route.path[i]->departureTime)
        printf("\nViolated time window at point %d in route %d", i, route.vehicle.id);
      else if (route.load[i] > route.vehicle.capacity)
        printf("\nViolated load at point %d in route %d", i, route.vehicle.id);
      else if (route.ridingTimes[i] > route.path[i]->maxRideTime)
        printf("\nViolated ride time at point %d in route %d", i, route.vehicle.id);
      else if (route.batteryLevels[i] < 0.0 || route.batteryLevels[i] > route.vehicle.batteryCapacity)
        printf("\nViolated battery levels at point %d in route %d", i, route.vehicle.id);
      else if (route.batteryLevels[route.path.size() - 1] < route.vehicle.batteryCapacity * route.vehicle.minFinalBatteryRatioLevel)
        printf("\nViolated final battery level at point %d in route %d", i, route.vehicle.id);
    }
  }

  if (solution.isFeasible())
    printf("Viável\nCusto = %.2f\nt = %.2fs\n", solution.cost, elapsed);
  else
    printf("Inviável\nCusto = %.2f [load = %d, timeWindow = %.2f, maxRideTime = %.2f]\nt = %.2fs\n",
            solution.cost, solution.loadViolation, solution.timeWindowViolation, solution.maxRideTimeViolation, elapsed);

  Gnuplot::plotSolution(solution);

  return EXIT_SUCCESS;
}

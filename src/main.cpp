/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>

#include "data-structures/Singleton.hpp"
#include "algorithms/Grasp.hpp"

#define MIN_ARGS_AMOUNT 1

bool isFeasible(Solution s)
{
  for (Route *r : s.routes) {
    for (int i = 0; i < r->path.size(); i++) {
      if (r->serviceBeginningTimes[i] > r->path[i]->departureTime) {
        printf("Time window violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->path[i]->isPickup() && r->ridingTimes[i] > r->path[i]->maxRideTime) {
        printf("Riding time violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->load[i] > r->vehicle->capacity) {
        printf("Vehicle load violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->batteryLevels[i] < 0) {
        printf("Battery level violation at point %d in route %d\n", i, r->vehicle->id);
        return false;
      }

      if (r->path[i]->isStation() && r->load[i] != 0) {
        printf("Load violation at station %d in route %d\n", i, r->vehicle->id);
        return false;
      }
    }
  }

  return true;
}

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Singleton *instance = Singleton::getInstance();
  instance->init(argv[1]);

  Solution solution = Grasp::solve(1000, {0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.50});

  for (Route *route : solution.routes) {
    route->printPath();
    route->printSchedule();
    printf("\n");
  }

  std::cout << '\n' << isFeasible(solution) << " " << solution.cost << "\n";

  return 0;
}

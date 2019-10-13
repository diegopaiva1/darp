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

  Solution solution = Grasp::solve();

  for (Route *r : solution.routes) {
    r->printPath();
    r->printSchedule();
    printf("\n");
  }

  std::cout << '\n' << isFeasible(solution) << " " << solution.cost() << "\n";

  return 0;
}

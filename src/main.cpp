/**
 * @file    main.cpp
 * @author  Diego Paiva
 * @date    24/09/2019
 */

#include <iostream>
#include "data-structures/Instance.hpp"
#include "algorithms/InsertionHeuristic.hpp"

#define MIN_ARGS_AMOUNT 1

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Instance instance(argv[1]);
  Solution s = InsertionHeuristic::getSolution(instance);

  for (Route *r : s.routes) {
    r->printPath();
    printf("\n");
    r->printSchedule();
  }

  return 0;
}

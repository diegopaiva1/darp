/**
 * @file    main.cpp
 * @author  Diego Paiva e Silva
 * @date    24/09/2019
 */

#include <iostream>
#include "data-structures/Instance.hpp"

#define MIN_ARGS_AMOUNT 1

int main(int argc, char *argv[])
{
  int argsGiven = argc - 1;

  if (argsGiven < MIN_ARGS_AMOUNT) {
    printf("Error: expected at least %d argument(s) - %d given\n", MIN_ARGS_AMOUNT, argsGiven);
    return EXIT_FAILURE;
  }

  Instance instance(argv[1]);

  for (Node *n : instance.nodes) {
    printf("%d %4d\t %6.3f\t %6.3f\t %6d\t %6d\t %6d\t %6d\t %6d %6.3f\n",
    n->type, n->id, n->point->x, n->point->y, n->serviceTime,
    n->maxRideTime, n->load, n->arrivalTime, n->departureTime, n->rechargingRate);
  }

  printf("\n");

  for (Vehicle *v : instance.vehicles) {
    printf("%d %4d\t %6.3f\t %6.3f\t %6.3f\t %6.3f\n",
    v->id, v->capacity, v->batteryCapacity, v->initialBatteryLevel, v->minFinalBatteryRatioLevel, v->dischargingRate);
  }

  return 0;
}

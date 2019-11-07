/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#include "Route.hpp"

Route::Route(Vehicle vehicle)
{
  cost = 0.0;
  this->vehicle = vehicle;
}

Route::Route()
{
  cost = 0.0;
}

Route::~Route()
{
  // Empty destructor
}

// A solution is feasible only and if only there are no constraints violations
bool Route::isFeasible()
{
  return loadViolation == 0 && maxRideTimeViolation == 0 &&
         timeWindowViolation == 0 && finalBatteryViolation == 0 && !batteryLevelViolation;
}

void Route::printPath()
{
  printf("Rota %d: ", vehicle.id);
  for (Node *node : path)
    printf("%d ", node->id);
}

void Route::printSchedule()
{
  for (int i = 0; i < path.size(); i++) {
    printf("A[%02d] = %6.4g\t", i,  arrivalTimes[i]);
    printf("B[%02d] = %6.4g\t", i,  serviceBeginningTimes[i]);
    printf("D[%02d] = %6.4g\t", i,  departureTimes[i]);
    printf("W[%02d] = %.4g\t",  i,  waitingTimes[i]);
    printf("R[%02d] = %.4g\t",  i,  ridingTimes[i]);
    printf("Z[%02d] = %.4g\t",  i,  batteryLevels[i]);
    printf("E[%02d] = %.4g\t",  i,  chargingTimes[i]);
    printf("Q[%02d] = %d",      i,  load[i]);
    printf("\n");
  }
}

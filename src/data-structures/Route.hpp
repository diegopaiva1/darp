/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#ifndef ROUTE_H_INCLUDED
#define ROUTE_H_INCLUDED

#include "Vehicle.hpp"
#include "Request.hpp"

#include <limits>
#include <algorithm>

class Route
{
public:
  Vehicle *vehicle;
  std::vector<Node *> path;
  std::vector<int> load;
  std::vector<float> arrivalTimes;
  std::vector<float> serviceBeginningTimes;
  std::vector<float> departureTimes;
  std::vector<float> waitingTimes;
  std::vector<float> ridingTimes;
  std::vector<float> batteryLevels;
  std::vector<float> chargingTimes;
  int loadViolation;
  float maxRideTimeViolation;
  float timeWindowViolation;
  bool batteryLevelViolation;
  float cost;


  Route(Vehicle *vehicle)
  {
    cost = 0.0;
    this->vehicle = vehicle;
  }

  Route()
  {
    cost = 0.0;
  }

  ~Route() {}

  // A solution is feasible only and if only there are no constraints violations
  bool isFeasible()
  {
    return loadViolation == 0 && maxRideTimeViolation == 0 && timeWindowViolation == 0 /*&& !batteryLevelViolation*/;
  }

  void printPath()
  {
    printf("Rota %d: ", vehicle->id);
    for (Node *node : path)
      printf("%d ", node->id);
    printf("\n");
  }

  void printSchedule()
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
};

#endif // ROUTE_H_INCLUDED

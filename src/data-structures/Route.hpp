/**
 * @file    Route.hpp
 * @author  Diego Paiva
 * @date    06/10/2019
 */

#ifndef ROUTE_HPP_INCLUDED
#define ROUTE_HPP_INCLUDED

#include "Vehicle.hpp"
#include "Request.hpp"

#include <iostream>
#include <vector>

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
  float finalBatteryViolation;
  bool batteryLevelViolation;
  float cost;

  Route(Vehicle *vehicle);

  Route();

  ~Route();

  // A solution is feasible only and if only there are no constraints violations
  bool isFeasible();

  void printPath();

  void printSchedule();
};

#endif // ROUTE_HPP_INCLUDED

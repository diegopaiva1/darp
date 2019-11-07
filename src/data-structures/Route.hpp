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
  Vehicle vehicle;
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

  Route(Vehicle vehicle);

  Route();

  ~Route();

 /**
  * @brief Checks if current solution is feasible (no constraints violations).
  *
  * @return True if it is feasible, false otherwise.
  */
  bool isFeasible();

 /**
  * @brief Print the nodes sequentially.
  */
  void printPath();

 /**
  * @brief Print arrival times, service beginning times, departure times, waiting times,
  *        riding times, load, battery levels and charging times for every index in this route's path.
  */
  void printSchedule();
};

#endif // ROUTE_HPP_INCLUDED

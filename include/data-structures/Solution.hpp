/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_HPP_INCLUDED
#define SOLUTION_HPP_INCLUDED

#include "Route.hpp"

class Solution
{
public:
  std::vector<Route> routes;
  int loadViolation;
  int orderViolation;
  bool batteryLevelViolation;
  double timeWindowViolation;
  double maxRideTimeViolation;
  double finalBatteryViolation;
  double travelTime;
  double excessRideTime;
  double cost;

  Solution();

  ~Solution();

  bool isFeasible();

  void computeCost();
};

#endif // SOLUTION_HPP_INCLUDED

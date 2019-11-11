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
  float cost;
  int loadViolation;
  float timeWindowViolation;
  float maxRideTimeViolation;
  bool  batteryLevelViolation;
  float finalBatteryViolation;

  Solution();

  ~Solution();

  bool isFeasible();

  void computeCost(std::vector<float> penaltyParams);
};

#endif // SOLUTION_HPP_INCLUDED

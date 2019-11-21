/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_HPP_INCLUDED
#define SOLUTION_HPP_INCLUDED

#define MAX_INT   std::numeric_limits<int>::max()
#define MAX_FLOAT std::numeric_limits<float>::max()

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

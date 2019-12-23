/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_HPP_INCLUDED
#define SOLUTION_HPP_INCLUDED

#include "Route.hpp"
#include "Singleton.hpp"

class Solution
{
public:
  std::vector<Route> routes;
  double cost;
  double timeWindowViolation;
  double maxRideTimeViolation;
  double finalBatteryViolation;
  bool batteryLevelViolation;
  int loadViolation;

  Solution();

  ~Solution();

  bool isFeasible();

  void computeCost(std::vector<double> penaltyParams);
};

#endif // SOLUTION_HPP_INCLUDED

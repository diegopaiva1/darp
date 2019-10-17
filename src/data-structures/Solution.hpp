/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_H_INCLUDED
#define SOLUTION_H_INCLUDED

#define MAX_INT   std::numeric_limits<int>::max()
#define MAX_FLOAT std::numeric_limits<float>::max()

#include "Route.hpp"

class Solution
{
public:
  std::vector<Route *> routes;
  float cost;
  int loadViolation;
  float timeWindowViolation;
  float maxRideTimeViolation;

  Solution()
  {
    cost                 = 0.0;
    loadViolation        = 0;
    timeWindowViolation  = 0.0;
    maxRideTimeViolation = 0.0;
  }

  ~Solution() { }

  bool isFeasible()
  {
    return loadViolation        == 0 &&
           timeWindowViolation  == 0 &&
           maxRideTimeViolation == 0 &&
           routes.size() <= Singleton::getInstance()->vehicles.size();
  }
};

#endif // SOLUTION_H_INCLUDED

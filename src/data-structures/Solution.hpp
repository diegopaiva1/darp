/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#ifndef SOLUTION_H_INCLUDED
#define SOLUTION_H_INCLUDED

#include "Route.hpp"

class Solution
{
public:
  std::vector<Route *> routes;
  float cost;

  Solution()
  {
    cost = 0.0;
  }

  ~Solution() { }
};

#endif // SOLUTION_H_INCLUDED

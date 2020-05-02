/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/Solution.hpp"
#include "data-structures/Singleton.hpp"

Solution::Solution()
{
  // Empty constructor
}

Solution::~Solution()
{
  // Empty destructor
}

bool Solution::feasible()
{
  return routes.size() <= inst->vehicles.size();
}

void Solution::updateCost()
{
  travelTime            = 0.0;
  excessRideTime        = 0.0;
  cost                  = 0.0;

  for (Route &r : routes) {
    travelTime     += r.travelTime;
    excessRideTime += r.excessRideTime;
    cost           += r.cost;
  }
}

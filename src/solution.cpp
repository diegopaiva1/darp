/**
 * @file   solution.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "solution.hpp"
#include "instance.hpp"

#include <cfloat>

Solution::Solution()
{
  cost = 0.0;
}

void Solution::add_route(Route r)
{
  // In case we are updating the vehicle's route...
  if (routes.find(r.vehicle) != routes.end())
    cost -= routes[r.vehicle].cost;

  routes[r.vehicle] = r;
  cost += r.cost;
}

bool Solution::feasible()
{
  return routes.size() <= inst.vehicles.size();
}

void Solution::delete_empty_routes()
{
  for (auto it = routes.begin(); it != routes.end(); )
    if (it->second.empty())
      it = routes.erase(it);
    else
      ++it;
}

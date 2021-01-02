/**
 * @file   solution.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "solution.hpp"
#include "instance.hpp"

#include <cmath>

void Solution::add_route(Route r)
{
  routes[r.vehicle] = r;
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

double Solution::obj_func_value()
{
  double sum = 0.0;

  for (auto [vehicle, route] : routes)
    sum += route.cost;

  return sum;
}

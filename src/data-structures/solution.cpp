/**
 * @file   solution.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "data-structures/solution.hpp"
#include "data-structures/instance.hpp"

bool Solution::feasible()
{
  for (Route r : routes)
    if (!r.feasible())
      return false;

  return true;
}

void Solution::set_route(Vehicle *&v, Route r)
{
  routes[v->id - 1] = r;
}

double Solution::obj_func_value()
{
  double sum = 0.0;

  for (Route r : routes)
    sum += r.cost;

  return sum;
}

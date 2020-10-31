/**
 * @file   solution.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "solution.hpp"
#include "instance.hpp"

#include <cmath>

bool Solution::feasible()
{
  for (Route r : routes)
    if (!r.feasible())
      return false;

  return !routes.empty() ? true : false;
}

void Solution::set_route(Vehicle *&v, Route r)
{
  routes[v->id - 1] = r;
}

double Solution::obj_func_value()
{
  if (routes.empty())
    return MAXFLOAT;

  double sum = 0.0;

  for (Route &r : routes)
    sum += r.cost;

  return sum;
}

std::string Solution::to_string()
{
  std::string s;

  for (Route &r : routes)
    s.append('\n' + r.to_string());

  return s;
}

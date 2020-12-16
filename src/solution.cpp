/**
 * @file   solution.cpp
 * @author Diego Paiva
 * @date   26/09/2019
 */

#include "solution.hpp"
#include "instance.hpp"

#include <cmath>

// void Solution::add_tour(Vehicle *v, Route r)
// {
//   routes[v] = r;
// }

void Solution::set_route(Vehicle *v, Route r)
{
  routes[v->id - 1] = r;
}

bool Solution::feasible()
{
  return routes.size() <= inst.vehicles.size();
}

void Solution::delete_empty_routes()
{
  for (auto r = routes.begin(); r != routes.end(); )
    r = r->empty() ? routes.erase(r) : r + 1;
}

double Solution::obj_func_value()
{
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

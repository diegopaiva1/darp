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
  for (Route r : routes)
    if (!r.feasible())
      return false;

  return true;
}

void Solution::setRoute(Vehicle *v, Route r)
{
  routes[v->id - 1] = r;
}

Route Solution::getRoute(Vehicle *v)
{
  return routes[v->id - 1];
}

double Solution::objFuncValue()
{
  double sum = 0.0;

  for (Route r : routes)
    sum += r.cost;

  return sum;
}

double Solution::travelTime()
{
  double sum = 0.0;

  for (Route r : routes)
    sum += r.travelTime;

  return sum;
}

double Solution::excessRideTime()
{
  double sum = 0.0;

  for (Route r : routes)
    sum += r.excessRideTime;

  return sum;
}

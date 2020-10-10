/**
 * @file   solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 *
 * A class to represent a solution for the DARP.
 */

#ifndef SOLUTION_HPP_INCLUDED
#define SOLUTION_HPP_INCLUDED

#include "route.hpp"

class Solution
{
public:
  std::vector<Route> routes;

 /**
  * Default constructor.
  */
  Solution() {};

 /**
  * Default destructor.
  */
  ~Solution() {};

 /**
  * Check if solution is feasible.
  *
  * @return `true` if feasible.
  */
  bool feasible();

 /**
  * Set the route traversed by a vehicle.
  *
  * @param v A vehicle.
  * @param r A route.
  */
  void set_route(Vehicle *&v, Route r);

 /**
  * Get Solution's objective function value.
  *
  * @return Objective function value.
  */
  double obj_func_value();
};

#endif // SOLUTION_HPP_INCLUDED

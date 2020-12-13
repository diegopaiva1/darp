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
  bool is_feasible;

 /**
  * Default constructor.
  */
  Solution() {};

 /**
  * Default destructor.
  */
  ~Solution() {};

 /**
  * Set the route traversed by a vehicle.
  *
  * @param v A vehicle.
  * @param r A route.
  */
  void set_route(Vehicle *v, Route r);

 /**
  * Delete routes without requests accomodated.
  */
  void delete_empty_routes();

 /**
  * Get Solution's objective function value.
  *
  * @return Objective function value.
  */
  double obj_func_value();

 /**
  * Convert Solution to string representation.
  */
  std::string to_string();
};

#endif // SOLUTION_HPP_INCLUDED

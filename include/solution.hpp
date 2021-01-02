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

#include <unordered_map>

class Solution
{
public:
  std::unordered_map<Vehicle*, Route> routes;

 /**
  * Add (or update) the route traversed by vehicle of route `r`.
  *
  * @param r Route.
  */
  void add_route(Route r);

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
  * Delete routes without requests accomodated.
  */
  void delete_empty_routes();

 /**
  * Get Solution's objective function value.
  *
  * @return Objective function value.
  */
  double obj_func_value();
};

#endif // SOLUTION_HPP_INCLUDED

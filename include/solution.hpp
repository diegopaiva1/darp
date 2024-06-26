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
 /*
  * Hash map to assign each vehicle to a particular route.
  */
  std::unordered_map<Vehicle*, Route> routes;

 /*
  * Solution's total cost.
  */
  double cost;

 /**
  * Add (or update) the route traversed by vehicle of route `r`.
  *
  * @param r Route.
  */
  void add_route(Route r);

 /**
  * Default constructor.
  */
  Solution();

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

 /**
  * Convert solution to a unique key representation.
  *
  * @return Unique key string.
  */
  std::string to_key();
};

#endif // SOLUTION_HPP_INCLUDED

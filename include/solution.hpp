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
  // std::vector<Route> routes;

  void add_route(Vehicle *v, Route r);

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

//  /**
//   * Set the route traversed by a vehicle.
//   *
//   * @param v A vehicle.
//   * @param r A route.
//   */
//   void set_route(Vehicle *v, Route r);

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

/**
 * @file   Solution.hpp
 * @author Diego Paiva
 * @date   26/09/2019
 *
 * A class to represent a solution for the e-ADARP.
 */

#ifndef SOLUTION_HPP_INCLUDED
#define SOLUTION_HPP_INCLUDED

#include "Route.hpp"

class Solution
{
public:
  std::vector<Route> routes;
  double travelTime;
  double excessRideTime;
  double cost;

 /**
  * @brief Default constructor.
  */
  Solution();

 /**
  * @brief Default destructor.
  */
  ~Solution();

 /**
  * @brief Check if solution is feasible.
  *
  * @return True if feasible.
  */
  bool feasible();

 /**
  * @brief Update solution's cost variables.
  */
  void updateCost();

 /**
  * @brief Set the route traversed by a vehicle.
  *
  * @param v A vehicle.
  * @param r A route.
  */
  void setRoute(Vehicle v, Route r);

 /**
  * @brief Get the route traversed by a given vehicle.
  *
  * @param v A vehicle.
  * @return  Route traversed by 'v'.
  */
  Route getRoute(Vehicle v);
};

#endif // SOLUTION_HPP_INCLUDED
